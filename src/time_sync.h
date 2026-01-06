#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "esp_err.h"

#define TIME_SYNC_NTP_POOL_MAX_LEN 64

typedef struct {
    char ntp_pool[TIME_SYNC_NTP_POOL_MAX_LEN];
    int32_t tz_offset_seconds;
} time_sync_config_t;

esp_err_t time_sync_init(void);
const time_sync_config_t *time_sync_get_config(void);
bool time_sync_get_local_time(struct tm *out_tm, time_t *out_epoch);

/* Pour future configuration Web/écran. */
esp_err_t time_sync_set_ntp_pool(const char *pool);
esp_err_t time_sync_set_tz_offset_seconds(int32_t offset_seconds);
esp_err_t time_sync_save_config(void);
