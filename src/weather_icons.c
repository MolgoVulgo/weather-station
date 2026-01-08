#include "weather_icons.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "svg2bin_decoder.h"
#include "ui_backend.h"

static const char *TAG = "WeatherIcons";

typedef struct {
    lv_img_dsc_t dsc;
    uint8_t *data;
    lv_obj_t *target;
} weather_icon_ctx_t;

static weather_icon_ctx_t s_icon_slots[8];

static weather_icon_ctx_t *weather_icon_ctx_for_target(lv_obj_t *target)
{
    if (!target) {
        return NULL;
    }
    for (size_t i = 0; i < sizeof(s_icon_slots) / sizeof(s_icon_slots[0]); ++i) {
        if (s_icon_slots[i].target == target) {
            return &s_icon_slots[i];
        }
    }
    for (size_t i = 0; i < sizeof(s_icon_slots) / sizeof(s_icon_slots[0]); ++i) {
        if (s_icon_slots[i].target == NULL) {
            s_icon_slots[i].target = target;
            return &s_icon_slots[i];
        }
    }
    s_icon_slots[0].target = target;
    return &s_icon_slots[0];
}

static void rgb565_swap(uint8_t *buf, size_t len)
{
    for (size_t i = 0; i + 1 < len; i += 2) {
        uint8_t tmp = buf[i];
        buf[i] = buf[i + 1];
        buf[i + 1] = tmp;
    }
}

static esp_err_t weather_icon_draw_cb(
    void *user_ctx,
    const char *name,
    uint16_t width,
    uint16_t height,
    const uint8_t *rgb565,
    size_t rgb565_len)
{
    (void)name;
    weather_icon_ctx_t *ctx = (weather_icon_ctx_t *)user_ctx;
    if (!ctx || !ctx->target || !rgb565) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t *copy = (uint8_t *)malloc(rgb565_len);
    if (!copy) {
        return ESP_ERR_NO_MEM;
    }
    memcpy(copy, rgb565, rgb565_len);

    if (ctx->data) {
        free(ctx->data);
    }
    ctx->data = copy;

#if LV_COLOR_16_SWAP
    rgb565_swap(ctx->data, rgb565_len);
#endif

    memset(&ctx->dsc, 0, sizeof(ctx->dsc));
    ctx->dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
    ctx->dsc.header.w = width;
    ctx->dsc.header.h = height;
    ctx->dsc.data_size = rgb565_len;
    ctx->dsc.data = ctx->data;

    lv_img_set_src(ctx->target, &ctx->dsc);
    return ESP_OK;
}

static uint16_t weather_icon_group_fallback(uint16_t code)
{
    if (code >= 200 && code <= 232) {
        return 200;
    }
    if (code >= 300 && code <= 321) {
        return 300;
    }
    if (code >= 500 && code <= 531) {
        return 500;
    }
    if (code >= 600 && code <= 622) {
        return 600;
    }
    if (code >= 700 && code <= 781) {
        return 701;
    }
    if (code == 800) {
        return 800;
    }
    if (code >= 801 && code <= 804) {
        return 801;
    }
    return 0;
}

static esp_err_t weather_icon_find_offset(FILE *fp,
                                          uint16_t code,
                                          uint8_t variant,
                                          uint32_t *out_offset)
{
    esp_err_t rc = svg2bin_find_entry_offset_stream(fp, code, variant, out_offset);
    if (rc == ESP_ERR_NOT_FOUND && variant != SVG2BIN_VARIANT_NEUTRAL) {
        rc = svg2bin_find_entry_offset_stream(fp, code, SVG2BIN_VARIANT_NEUTRAL, out_offset);
    }
    return rc;
}

static FILE *open_weather_bin(const char *filename)
{
    static const char *paths[] = {
        "/sdcard",
        "/spiffs",
    };

    for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
        char full_path[64];
        int written = snprintf(full_path, sizeof(full_path), "%s/%s", paths[i], filename);
        if (written <= 0 || (size_t)written >= sizeof(full_path)) {
            continue;
        }
        FILE *fp = fopen(full_path, "rb");
        if (fp) {
            ESP_LOGI(TAG, "Using weather bin: %s", full_path);
            return fp;
        }
    }

    ESP_LOGE(TAG, "Weather bin not found on SD or SPIFFS: %s", filename);
    return NULL;
}

esp_err_t weather_icons_set_object(lv_obj_t *target,
                                   const char *bin_name,
                                   uint16_t code,
                                   uint8_t variant)
{
    if (!target) {
        ESP_LOGE(TAG, "Weather target not ready.");
        return ESP_ERR_INVALID_STATE;
    }

    const char *bin = bin_name ? bin_name : "icon_150.bin";
    FILE *fp = open_weather_bin(bin);
    if (!fp) {
        return ESP_ERR_NOT_FOUND;
    }

    uint32_t offset = 0;
    esp_err_t rc = weather_icon_find_offset(fp, code, variant, &offset);
    if (rc == ESP_ERR_NOT_FOUND) {
        uint16_t fallback = weather_icon_group_fallback(code);
        if (fallback != 0 && fallback != code) {
            ESP_LOGW(TAG, "Icon fallback: code=%u -> %u", (unsigned)code, (unsigned)fallback);
            rc = weather_icon_find_offset(fp, fallback, variant, &offset);
            if (rc == ESP_OK) {
                code = fallback;
            }
        }
    }
    if (rc != ESP_OK) {
        ESP_LOGE(TAG, "Index lookup failed (code=%u, variant=%u): %s",
                 (unsigned)code, (unsigned)variant, esp_err_to_name(rc));
        fclose(fp);
        return rc;
    }

    weather_icon_ctx_t *ctx = weather_icon_ctx_for_target(target);
    if (!ctx) {
        fclose(fp);
        return ESP_ERR_NO_MEM;
    }
    rc = svg2bin_decode_entry_at_offset(fp, offset, weather_icon_draw_cb, ctx);
    fclose(fp);
    if (rc != ESP_OK) {
        ESP_LOGE(TAG, "Decode failed: %s", esp_err_to_name(rc));
        return rc;
    }

    ESP_LOGI(TAG, "Icon set: code=%u variant=%u", (unsigned)code, (unsigned)variant);
    return ESP_OK;
}

esp_err_t weather_icons_set_main(uint16_t code, uint8_t variant)
{
    return weather_icons_set_object(ui_weather_image(), "icon_150.bin", code, variant);
}
