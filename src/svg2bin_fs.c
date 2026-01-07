#include "svg2bin_fs.h"

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include <esp_spiffs.h>

static const char *TAG = "Svg2BinFS";

esp_err_t svg2bin_fs_init_spiffs(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false,
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPIFFS mount failed: %s", esp_err_to_name(ret));
        return ret;
    }

    size_t total = 0;
    size_t used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "SPIFFS total=%u used=%u", (unsigned)total, (unsigned)used);
    }
    return ESP_OK;
}

static const char *svg2bin_base_path(svg2bin_storage_t storage)
{
    switch (storage) {
    case SVG2BIN_STORAGE_SD:
        return "/sdcard";
    case SVG2BIN_STORAGE_SPIFFS:
    default:
        return "/spiffs";
    }
}

esp_err_t svg2bin_fs_read_file(svg2bin_storage_t storage, const char *path,
                               uint8_t **out_buf, size_t *out_len)
{
    if (!path || !out_buf || !out_len) {
        return ESP_ERR_INVALID_ARG;
    }

    char full_path[256];
    int written = snprintf(full_path, sizeof(full_path), "%s/%s",
                           svg2bin_base_path(storage), path);
    if (written <= 0 || (size_t)written >= sizeof(full_path)) {
        return ESP_ERR_INVALID_SIZE;
    }

    FILE *fp = fopen(full_path, "rb");
    if (!fp) {
        ESP_LOGE(TAG, "Open failed: %s", full_path);
        return ESP_FAIL;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return ESP_FAIL;
    }
    long len = ftell(fp);
    if (len <= 0) {
        fclose(fp);
        return ESP_ERR_INVALID_SIZE;
    }
    rewind(fp);

    uint8_t *buf = (uint8_t *)malloc((size_t)len);
    if (!buf) {
        fclose(fp);
        return ESP_ERR_NO_MEM;
    }

    size_t read = fread(buf, 1, (size_t)len, fp);
    fclose(fp);
    if (read != (size_t)len) {
        free(buf);
        return ESP_FAIL;
    }

    *out_buf = buf;
    *out_len = (size_t)len;
    ESP_LOGI(TAG, "Loaded %s (%u bytes)", full_path, (unsigned)len);
    return ESP_OK;
}
