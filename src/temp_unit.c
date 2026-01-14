#include "temp_unit.h"

#include <stdio.h>
#include "esp_log.h"
#include "nvs.h"

#define TEMP_UNIT_NVS_NAMESPACE "weather_cfg"
#define TEMP_UNIT_NVS_KEY "temp_unit"

static const char *TAG = "TempUnit";

static bool s_temp_unit_f = false;
static bool s_has_last_temp = false;
static float s_last_temp_c = 0.0f;

void temp_unit_init(void)
{
    nvs_handle_t nvs = 0;
    esp_err_t ret = nvs_open(TEMP_UNIT_NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed: %s", esp_err_to_name(ret));
        return;
    }

    uint8_t stored = 0;
    ret = nvs_get_u8(nvs, TEMP_UNIT_NVS_KEY, &stored);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        stored = 0;
        ret = nvs_set_u8(nvs, TEMP_UNIT_NVS_KEY, stored);
        if (ret == ESP_OK) {
            ret = nvs_commit(nvs);
        }
    } else if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Lecture NVS temp_unit échouée: %s", esp_err_to_name(ret));
    }
    s_temp_unit_f = (stored != 0);
    nvs_close(nvs);
}

bool temp_unit_is_fahrenheit(void)
{
    return s_temp_unit_f;
}

void temp_unit_set_fahrenheit(bool is_fahrenheit)
{
    s_temp_unit_f = is_fahrenheit;

    nvs_handle_t nvs = 0;
    esp_err_t ret = nvs_open(TEMP_UNIT_NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = nvs_set_u8(nvs, TEMP_UNIT_NVS_KEY, is_fahrenheit ? 1 : 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS set temp_unit failed: %s", esp_err_to_name(ret));
        nvs_close(nvs);
        return;
    }
    ret = nvs_commit(nvs);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS commit failed: %s", esp_err_to_name(ret));
    }
    nvs_close(nvs);
}

void temp_unit_set_last_c(float temp_c)
{
    s_last_temp_c = temp_c;
    s_has_last_temp = true;
}

bool temp_unit_has_last(void)
{
    return s_has_last_temp;
}

float temp_unit_get_last_c(void)
{
    return s_last_temp_c;
}

void temp_unit_format(float temp_c, char *out, size_t out_len)
{
    if (!out || out_len == 0) {
        return;
    }
    float display = temp_c;
    const char *unit = "°C";
    if (s_temp_unit_f) {
        display = (temp_c * 9.0f / 5.0f) + 32.0f;
        unit = "°F";
    }
    snprintf(out, out_len, "%.1f%s", display, unit);
    out[out_len - 1] = 0;
}

void temp_unit_format_range(float temp_min_c, float temp_max_c, char *out, size_t out_len)
{
    if (!out || out_len == 0) {
        return;
    }
    float min_display = temp_min_c;
    float max_display = temp_max_c;
    const char *unit = "°C";
    if (s_temp_unit_f) {
        min_display = (temp_min_c * 9.0f / 5.0f) + 32.0f;
        max_display = (temp_max_c * 9.0f / 5.0f) + 32.0f;
        unit = "°F";
    }
    snprintf(out, out_len, "%.0f/%.0f%s", min_display, max_display, unit);
    out[out_len - 1] = 0;
}
