#include "weather_service.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "esp_bsp.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "secrets.h"
#include "ui_backend.h"
#include "vars.h"
#include "boot_progress.h"
#include "ui/screens.h"
#include "temp_unit.h"
#include "weather_cache.h"
#include "weather_fetcher.h"
#include "weather_icons.h"

static const char *TAG = "WeatherService";
static esp_timer_handle_t s_weather_timer;
static esp_timer_handle_t s_weather_once_timer;
static bool s_weather_started;
static bool s_first_fetch_done;
static esp_event_handler_instance_t s_ip_handler;
static TaskHandle_t s_weather_task;
static const char *kWeekdaysShort[7] = {"DIM", "LUN", "MAR", "MER", "JEU", "VEN", "SAM"};
static bool s_boot_done;
static bool s_keys_loaded;
static char s_api_key_2[64];
static char s_api_key_3[64];
static ForecastEntry s_last_forecast[6];
static size_t s_last_forecast_count;
static bool s_last_forecast_valid;
static bool s_cache_ready;

#define WEATHER_JSON_MAX_BYTES (100 * 1024)

static esp_err_t weather_fetch_json_cached(const char *url, std::string &fallback, const char **out_json)
{
    *out_json = NULL;
    if (!url || !*url) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!s_cache_ready) {
        s_cache_ready = weather_cache_init(WEATHER_JSON_MAX_BYTES);
        if (!s_cache_ready) {
            ESP_LOGW(TAG, "PSRAM cache indisponible, fallback heap");
        }
    }

    if (s_cache_ready) {
        weather_cache_reset();
    }
    fallback.clear();

    esp_http_client_config_t config = {};
    config.url = url;
    config.timeout_ms = 8000;
    config.skip_cert_common_name_check = true;
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
    config.crt_bundle_attach = esp_crt_bundle_attach;
#endif

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "HTTP init failed");
        return ESP_FAIL;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP open failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return err;
    }

    int status = esp_http_client_fetch_headers(client);
    if (status < 0) {
        ESP_LOGE(TAG, "HTTP headers invalides");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    int http_code = esp_http_client_get_status_code(client);
    if (http_code != 200) {
        ESP_LOGE(TAG, "HTTP code %d", http_code);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    char buffer[512];
    int read_len = 0;
    bool overflowed = false;
    while ((read_len = esp_http_client_read(client, buffer, sizeof(buffer))) > 0) {
        if (s_cache_ready && !overflowed) {
            if (!weather_cache_write(buffer, (size_t)read_len)) {
                overflowed = true;
                size_t cached_len = 0;
                const char *cached = weather_cache_data(&cached_len);
                if (cached && cached_len > 0) {
                    fallback.assign(cached, cached_len);
                }
                fallback.append(buffer, (size_t)read_len);
            }
        } else {
            fallback.append(buffer, (size_t)read_len);
        }
    }

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    if (read_len < 0) {
        ESP_LOGE(TAG, "Lecture HTTP incomplete");
        return ESP_FAIL;
    }

    if (overflowed || !s_cache_ready) {
        if (overflowed) {
            ESP_LOGW(TAG, "JSON OWM > %u bytes, fallback heap", (unsigned)WEATHER_JSON_MAX_BYTES);
        }
        *out_json = fallback.c_str();
    } else {
        *out_json = weather_cache_data(NULL);
    }
    return ESP_OK;
}

#define WEATHER_NVS_NAMESPACE "weather_cfg"
#define WEATHER_NVS_KEY_API2 "api_key_2"
#define WEATHER_NVS_KEY_API3 "api_key_3"

static void weather_load_api_keys(void)
{
    if (s_keys_loaded) {
        return;
    }
    s_keys_loaded = true;
    s_api_key_2[0] = '\0';
    s_api_key_3[0] = '\0';

    nvs_handle_t nvs = 0;
    esp_err_t ret = nvs_open(WEATHER_NVS_NAMESPACE, NVS_READONLY, &nvs);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "NVS open weather failed: %s", esp_err_to_name(ret));
        return;
    }

    size_t len = sizeof(s_api_key_2);
    ret = nvs_get_str(nvs, WEATHER_NVS_KEY_API2, s_api_key_2, &len);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        s_api_key_2[0] = '\0';
    } else if (ret != ESP_OK) {
        ESP_LOGW(TAG, "NVS read api_key_2 failed: %s", esp_err_to_name(ret));
        s_api_key_2[0] = '\0';
    }

    len = sizeof(s_api_key_3);
    ret = nvs_get_str(nvs, WEATHER_NVS_KEY_API3, s_api_key_3, &len);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        s_api_key_3[0] = '\0';
    } else if (ret != ESP_OK) {
        ESP_LOGW(TAG, "NVS read api_key_3 failed: %s", esp_err_to_name(ret));
        s_api_key_3[0] = '\0';
    }

    nvs_close(nvs);
}

static const char *weather_api_key_2(void)
{
    weather_load_api_keys();
    return (s_api_key_2[0] != '\0') ? s_api_key_2 : OPENWEATHERMAP_API_KEY_2;
}

static const char *weather_api_key_3(void)
{
    weather_load_api_keys();
    return (s_api_key_3[0] != '\0') ? s_api_key_3 : OPENWEATHERMAP_API_KEY_3;
}

static bool weather_netif_ready(void)
{
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (!netif) {
        return false;
    }
    esp_netif_ip_info_t ip_info;
    if (esp_netif_get_ip_info(netif, &ip_info) != ESP_OK) {
        return false;
    }
    return ip_info.ip.addr != 0;
}

static void weather_apply_ui(const CurrentWeatherData *current)
{
    if (!current) {
        return;
    }
    bsp_display_lock(0);
    char temp_buf[16];
    temp_unit_set_last_c(current->temperature);
    temp_unit_format(current->temperature, temp_buf, sizeof(temp_buf));
    if (temp_buf[0] != '\0') {
        set_var_ui_meteo_temp(temp_buf);
    }
    if (!current->description.empty()) {
        set_var_ui_meteo_condition(current->description.c_str());
    }
    esp_err_t icon_ret = weather_icons_set_main(
        (uint16_t)current->conditionId, current->iconVariant);
    if (icon_ret != ESP_OK) {
        ESP_LOGE(TAG, "Weather icon set failed: %s", esp_err_to_name(icon_ret));
    }
    tick_screen_by_id(SCREEN_ID_UI_METEO);
    bsp_display_unlock();
}

static void weather_apply_forecast(const ForecastEntry *entries, size_t count)
{
    if (!entries || count == 0) {
        return;
    }
    size_t copy_count = count < 6 ? count : 6;
    for (size_t i = 0; i < copy_count; ++i) {
        s_last_forecast[i] = entries[i];
    }
    s_last_forecast_count = copy_count;
    s_last_forecast_valid = true;

    bsp_display_lock(0);
    for (size_t i = 0; i < 6 && i < count; ++i) {
        const ForecastEntry *entry = &entries[i];
        if (!entry->valid || entry->timestamp == 0) {
            continue;
        }
        struct tm day_tm;
        if (localtime_r(&entry->timestamp, &day_tm)) {
            const char *day = "";
            if (day_tm.tm_wday >= 0 && day_tm.tm_wday < 7) {
                day = kWeekdaysShort[day_tm.tm_wday];
            }
            switch (i) {
            case 0:
                set_var_ui_meteo_fd1(day);
                break;
            case 1:
                set_var_ui_meteo_fd2(day);
                break;
            case 2:
                set_var_ui_meteo_fd3(day);
                break;
            case 3:
                set_var_ui_meteo_fd4(day);
                break;
            case 4:
                set_var_ui_meteo_fd5(day);
                break;
            case 5:
                set_var_ui_meteo_fd6(day);
                break;
            default:
                break;
            }
        }

        char temp_range[24];
        temp_unit_format_range(entry->minTemp, entry->maxTemp, temp_range, sizeof(temp_range));
        switch (i) {
        case 0:
            set_var_ui_meteo_ft1(temp_range);
            break;
        case 1:
            set_var_ui_meteo_ft2(temp_range);
            break;
        case 2:
            set_var_ui_meteo_ft3(temp_range);
            break;
        case 3:
            set_var_ui_meteo_ft4(temp_range);
            break;
        case 4:
            set_var_ui_meteo_ft5(temp_range);
            break;
        case 5:
            set_var_ui_meteo_ft6(temp_range);
            break;
        default:
            break;
        }

        lv_obj_t *icon = ui_weather_forecast_icon(i);
        if (icon && entry->conditionId != 0) {
            weather_icons_set_object(
                icon,
                "icon_50.bin",
                (uint16_t)entry->conditionId,
                entry->iconVariant);
        }
    }
    tick_screen_by_id(SCREEN_ID_UI_METEO);
    bsp_display_unlock();
}

void weather_service_refresh_forecast_units(void)
{
    if (!s_last_forecast_valid || s_last_forecast_count == 0) {
        return;
    }
    weather_apply_forecast(s_last_forecast, s_last_forecast_count);
}

static void weather_fetch_once(void)
{
    if (!weather_netif_ready()) {
        return;
    }
    CurrentWeatherData current;
    ForecastEntry daily[6];
    char url[256];
    esp_err_t weather_ret = ESP_FAIL;
    std::string json_fallback;
    const char *json = NULL;
    const char *api_key_3 = weather_api_key_3();
    const char *api_key_2 = weather_api_key_2();
    if (strlen(api_key_3) > 0) {
        int written = snprintf(
            url,
            sizeof(url),
            "%s?lat=%.6f&lon=%.6f&appid=%s&units=metric&lang=%s",
            OPENWEATHERMAP_ONECALL_URL,
            LOCATION_LATITUDE,
            LOCATION_LONGITUDE,
            api_key_3,
            OPENWEATHERMAP_LANGUAGE);
        if (written <= 0 || (size_t)written >= sizeof(url)) {
            ESP_LOGE(TAG, "Weather URL overflow");
            return;
        }
        weather_ret = weather_fetch_json_cached(url, json_fallback, &json);
        if (weather_ret == ESP_OK) {
            if (!WeatherFetcher::parseOneCallJson(json, current, daily, 6, NULL, 0, NULL, 0)) {
                ESP_LOGE(TAG, "Parsing onecall incomplet");
                weather_ret = ESP_FAIL;
            }
        }
    } else {
        int written = snprintf(
            url,
            sizeof(url),
            "%s?lat=%.6f&lon=%.6f&appid=%s&units=metric&lang=%s",
            OPENWEATHERMAP_CURRENT_URL,
            LOCATION_LATITUDE,
            LOCATION_LONGITUDE,
            api_key_2,
            OPENWEATHERMAP_LANGUAGE);
        if (written <= 0 || (size_t)written >= sizeof(url)) {
            ESP_LOGE(TAG, "Weather URL overflow");
            return;
        }
        weather_ret = weather_fetch_json_cached(url, json_fallback, &json);
        if (weather_ret == ESP_OK) {
            if (!WeatherFetcher::parseCurrentJson(json, current)) {
                ESP_LOGE(TAG, "Parsing current incomplet");
                weather_ret = ESP_FAIL;
            }
        }
    }
    if (weather_ret != ESP_OK) {
        ESP_LOGE(TAG, "Weather fetch failed");
        return;
    }
    boot_progress_set(75, "Meteo");
    weather_apply_ui(&current);
    if (strlen(api_key_3) > 0) {
        weather_apply_forecast(daily, 6);
        if (!s_boot_done) {
            boot_progress_set(100, "Forecast");
            boot_progress_show_meteo();
            s_boot_done = true;
        }
    } else {
        ForecastEntry forecast[6];
        int written = snprintf(
            url,
            sizeof(url),
            "%s?lat=%.6f&lon=%.6f&appid=%s&units=metric&lang=%s",
            OPENWEATHERMAP_FORECAST_URL,
            LOCATION_LATITUDE,
            LOCATION_LONGITUDE,
            api_key_2,
            OPENWEATHERMAP_LANGUAGE);
        if (written <= 0 || (size_t)written >= sizeof(url)) {
            ESP_LOGE(TAG, "Forecast URL overflow");
            return;
        }
        json_fallback.clear();
        json = NULL;
        esp_err_t forecast_ret = weather_fetch_json_cached(url, json_fallback, &json);
        if (forecast_ret == ESP_OK) {
            if (!WeatherFetcher::parseForecastJson(json, forecast, 6)) {
                ESP_LOGE(TAG, "Parsing forecast incomplet");
                forecast_ret = ESP_FAIL;
            }
        }
        if (forecast_ret == ESP_OK) {
            weather_apply_forecast(forecast, 6);
            if (!s_boot_done) {
                boot_progress_set(100, "Forecast");
                boot_progress_show_meteo();
                s_boot_done = true;
            }
        } else {
            ESP_LOGW(TAG, "Forecast fetch failed");
        }
    }
    s_first_fetch_done = true;
}

static void weather_task(void *arg)
{
    (void)arg;
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        weather_fetch_once();
    }
}

static void weather_request_fetch(void)
{
    if (s_weather_task) {
        xTaskNotifyGive(s_weather_task);
    }
}

void weather_service_request_update(void)
{
    if (!s_weather_started) {
        ESP_LOGW(TAG, "Weather service non demarre");
        return;
    }
    weather_request_fetch();
}

static void weather_timer_cb(void *arg)
{
    (void)arg;
    weather_request_fetch();
}

static void weather_ip_event_handler(void *arg,
                                     esp_event_base_t event_base,
                                     int32_t event_id,
                                     void *event_data)
{
    (void)arg;
    (void)event_data;
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        if (!s_first_fetch_done) {
            ESP_LOGI(TAG, "WiFi IP obtenue, relance meteo");
            if (s_weather_once_timer) {
                esp_timer_stop(s_weather_once_timer);
                esp_timer_start_once(s_weather_once_timer, 500 * 1000);
            }
        }
    }
}

void weather_service_start(void)
{
    if (s_weather_started) {
        return;
    }
    s_weather_started = true;

    if (!s_weather_task) {
        const uint32_t stack_size = 8192;
        const UBaseType_t priority = tskIDLE_PRIORITY + 3;
        BaseType_t res = xTaskCreate(
            weather_task,
            "WeatherTask",
            stack_size,
            NULL,
            priority,
            &s_weather_task);
        if (res != pdPASS) {
            s_weather_task = NULL;
            ESP_LOGE(TAG, "Weather task create failed");
            return;
        }
    }

    const int64_t period_us = (int64_t)WEATHER_REFRESH_MINUTES * 60 * 1000000LL;
    esp_timer_create_args_t args = {
        .callback = &weather_timer_cb,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "weather_timer",
        .skip_unhandled_events = true,
    };
    esp_err_t err = esp_timer_create(&args, &s_weather_timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Timer create failed: %s", esp_err_to_name(err));
        return;
    }
    esp_timer_create_args_t once_args = {
        .callback = &weather_timer_cb,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "weather_once",
        .skip_unhandled_events = true,
    };
    err = esp_timer_create(&once_args, &s_weather_once_timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Timer once create failed: %s", esp_err_to_name(err));
        return;
    }
    err = esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &weather_ip_event_handler,
        NULL,
        &s_ip_handler);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "IP event handler failed: %s", esp_err_to_name(err));
        return;
    }
    err = esp_timer_start_periodic(s_weather_timer, period_us);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Timer start failed: %s", esp_err_to_name(err));
    }
}
