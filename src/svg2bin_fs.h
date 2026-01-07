#pragma once

#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

typedef enum {
    SVG2BIN_STORAGE_SPIFFS = 0,
    SVG2BIN_STORAGE_SD = 1,
} svg2bin_storage_t;

esp_err_t svg2bin_fs_init_spiffs(void);
esp_err_t svg2bin_fs_read_file(svg2bin_storage_t storage, const char *path,
                               uint8_t **out_buf, size_t *out_len);
