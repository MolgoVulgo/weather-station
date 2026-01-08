#include "weather_service.h"

#include <stdio.h>
#include <string.h>
#include "esp_bsp.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "secrets.h"
#include "ui_backend.h"
#include "vars.h"
#include "weather_fetcher.h"
#include "weather_icons.h"

static const char *TAG = "WeatherService";
static esp_timer_handle_t s_weather_timer;
static esp_timer_handle_t s_weather_once_timer;
static bool s_weather_started;
static bool s_first_fetch_done;
static esp_event_handler_instance_t s_ip_handler;
static TaskHandle_t s_weather_task;

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
    int temp_written = snprintf(temp_buf, sizeof(temp_buf), "%.1f", current->temperature);
    if (temp_written > 0) {
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
    bsp_display_unlock();
}

static void weather_fetch_once(void)
{
    if (!weather_netif_ready()) {
        ESP_LOGW(TAG, "WiFi pas encore connecte, meteo differee");
        return;
    }
    WeatherFetcher fetcher;
    CurrentWeatherData current;
    char url[256];
    esp_err_t weather_ret = ESP_FAIL;
    if (strlen(OPENWEATHERMAP_API_KEY_3) > 0) {
        int written = snprintf(
            url,
            sizeof(url),
            "%s?lat=%.6f&lon=%.6f&appid=%s&units=metric&lang=%s",
            OPENWEATHERMAP_ONECALL_URL,
            LOCATION_LATITUDE,
            LOCATION_LONGITUDE,
            OPENWEATHERMAP_API_KEY_3,
            OPENWEATHERMAP_LANGUAGE);
        if (written <= 0 || (size_t)written >= sizeof(url)) {
            ESP_LOGE(TAG, "Weather URL overflow");
            return;
        }
        weather_ret = fetcher.fetchOneCall(url, current, NULL, 0, NULL, 0, NULL, 0);
    } else {
        int written = snprintf(
            url,
            sizeof(url),
            "%s?lat=%.6f&lon=%.6f&appid=%s&units=metric&lang=%s",
            OPENWEATHERMAP_CURRENT_URL,
            LOCATION_LATITUDE,
            LOCATION_LONGITUDE,
            OPENWEATHERMAP_API_KEY_2,
            OPENWEATHERMAP_LANGUAGE);
        if (written <= 0 || (size_t)written >= sizeof(url)) {
            ESP_LOGE(TAG, "Weather URL overflow");
            return;
        }
        weather_ret = fetcher.fetchCurrent(url, current);
    }
    if (weather_ret != ESP_OK) {
        ESP_LOGE(TAG, "Weather fetch failed: %s", fetcher.lastError().c_str());
        return;
    }
    weather_apply_ui(&current);
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
    weather_fetch_once();
    err = esp_timer_start_periodic(s_weather_timer, period_us);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Timer start failed: %s", esp_err_to_name(err));
    }
}
