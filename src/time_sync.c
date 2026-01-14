#include "time_sync.h"

#include <string.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "TimeSync";

#define TIME_SYNC_NVS_NAMESPACE "time_cfg"
#define TIME_SYNC_NVS_KEY_POOL "ntp_pool"
#define TIME_SYNC_NVS_KEY_TZ_OFFSET "tz_offset"
#define TIME_SYNC_NVS_KEY_HOUR_FORMAT "hour_format"

#define TIME_SYNC_DEFAULT_NTP_POOL "pool.ntp.org"
#define TIME_SYNC_VALID_EPOCH_MIN 1609459200L

static time_sync_config_t time_sync_cfg = {
    .ntp_pool = TIME_SYNC_DEFAULT_NTP_POOL,
    .tz_offset_seconds = 0,
    .hour_format_24h = true,
};

static void time_sync_log_callback(struct timeval *tv)
{
    (void)tv;
    ESP_LOGI(TAG, "Heure synchronisée via NTP");
}

static esp_err_t time_sync_nvs_load(bool *out_missing)
{
    nvs_handle_t nvs = 0;
    esp_err_t ret = nvs_open(TIME_SYNC_NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed: %s", esp_err_to_name(ret));
        return ret;
    }

    bool missing = false;
    size_t len = sizeof(time_sync_cfg.ntp_pool);
    ret = nvs_get_str(nvs, TIME_SYNC_NVS_KEY_POOL, time_sync_cfg.ntp_pool, &len);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        missing = true;
        strlcpy(time_sync_cfg.ntp_pool, TIME_SYNC_DEFAULT_NTP_POOL, sizeof(time_sync_cfg.ntp_pool));
    } else if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Lecture NVS pool échouée: %s", esp_err_to_name(ret));
        strlcpy(time_sync_cfg.ntp_pool, TIME_SYNC_DEFAULT_NTP_POOL, sizeof(time_sync_cfg.ntp_pool));
    }

    int32_t tz_offset = 0;
    ret = nvs_get_i32(nvs, TIME_SYNC_NVS_KEY_TZ_OFFSET, &tz_offset);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        missing = true;
        tz_offset = 0;
    } else if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Lecture NVS décalage échouée: %s", esp_err_to_name(ret));
        tz_offset = 0;
    }
    time_sync_cfg.tz_offset_seconds = tz_offset;

    uint8_t hour_format = 1;
    ret = nvs_get_u8(nvs, TIME_SYNC_NVS_KEY_HOUR_FORMAT, &hour_format);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        missing = true;
        hour_format = 1;
    } else if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Lecture NVS format heure echouee: %s", esp_err_to_name(ret));
        hour_format = 1;
    }
    time_sync_cfg.hour_format_24h = (hour_format != 0);

    nvs_close(nvs);

    if (out_missing) {
        *out_missing = missing;
    }
    return ESP_OK;
}

esp_err_t time_sync_save_config(void)
{
    nvs_handle_t nvs = 0;
    esp_err_t ret = nvs_open(TIME_SYNC_NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_set_str(nvs, TIME_SYNC_NVS_KEY_POOL, time_sync_cfg.ntp_pool);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS set pool failed: %s", esp_err_to_name(ret));
        nvs_close(nvs);
        return ret;
    }

    ret = nvs_set_i32(nvs, TIME_SYNC_NVS_KEY_TZ_OFFSET, time_sync_cfg.tz_offset_seconds);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS set offset failed: %s", esp_err_to_name(ret));
        nvs_close(nvs);
        return ret;
    }

    ret = nvs_set_u8(nvs, TIME_SYNC_NVS_KEY_HOUR_FORMAT, time_sync_cfg.hour_format_24h ? 1 : 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS set format heure failed: %s", esp_err_to_name(ret));
        nvs_close(nvs);
        return ret;
    }

    ret = nvs_commit(nvs);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS commit failed: %s", esp_err_to_name(ret));
    }
    nvs_close(nvs);
    return ret;
}

esp_err_t time_sync_set_ntp_pool(const char *pool)
{
    if (!pool || pool[0] == '\0') {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(time_sync_cfg.ntp_pool, pool, sizeof(time_sync_cfg.ntp_pool));
    if (esp_sntp_enabled()) {
        esp_sntp_setservername(0, time_sync_cfg.ntp_pool);
    }
    return ESP_OK;
}

esp_err_t time_sync_set_tz_offset_seconds(int32_t offset_seconds)
{
    time_sync_cfg.tz_offset_seconds = offset_seconds;
    return ESP_OK;
}

esp_err_t time_sync_set_hour_format_24h(bool is_24h)
{
    time_sync_cfg.hour_format_24h = is_24h;
    return ESP_OK;
}

static void time_sync_start_sntp(void)
{
    if (esp_sntp_enabled()) {
        esp_sntp_stop();
    }

    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, time_sync_cfg.ntp_pool);
    esp_sntp_set_time_sync_notification_cb(time_sync_log_callback);
    esp_sntp_init();

    ESP_LOGI(TAG, "NTP actif: %s (offset %ld s)",
             time_sync_cfg.ntp_pool,
             (long)time_sync_cfg.tz_offset_seconds);
}

esp_err_t time_sync_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS erase requis, tentative d'effacement");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_netif_init();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_netif_init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "event loop init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    bool missing = false;
    ret = time_sync_nvs_load(&missing);
    if (ret != ESP_OK) {
        return ret;
    }
    if (missing) {
        ESP_LOGI(TAG, "NVS time_cfg absent, sauvegarde des valeurs par défaut");
        time_sync_save_config();
    }

    time_sync_start_sntp();
    return ESP_OK;
}

const time_sync_config_t *time_sync_get_config(void)
{
    return &time_sync_cfg;
}

bool time_sync_get_local_time(struct tm *out_tm, time_t *out_epoch)
{
    if (!out_tm) {
        return false;
    }

    time_t now = 0;
    time(&now);
    if (now < TIME_SYNC_VALID_EPOCH_MIN) {
        return false;
    }

    time_t adjusted = now + (time_t)time_sync_cfg.tz_offset_seconds;
    gmtime_r(&adjusted, out_tm);
    if (out_epoch) {
        *out_epoch = adjusted;
    }
    return true;
}
