#include "weather_cache.h"

#include <string.h>
#include "esp_heap_caps.h"
#include "esp_log.h"

static const char *TAG = "WeatherCache";

static char *s_buffer = NULL;
static size_t s_capacity = 0;
static size_t s_length = 0;
static bool s_overflowed = false;

bool weather_cache_init(size_t max_size)
{
    if (s_buffer) {
        return true;
    }
    s_buffer = (char *)heap_caps_malloc(max_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!s_buffer) {
        ESP_LOGE(TAG, "PSRAM alloc failed (%u bytes)", (unsigned)max_size);
        return false;
    }
    s_capacity = max_size;
    s_length = 0;
    s_overflowed = false;
    s_buffer[0] = '\0';
    return true;
}

void weather_cache_reset(void)
{
    s_length = 0;
    s_overflowed = false;
    if (s_buffer) {
        s_buffer[0] = '\0';
    }
}

bool weather_cache_write(const char *data, size_t len)
{
    if (!s_buffer || !data || len == 0) {
        return false;
    }
    if (s_length + len + 1 > s_capacity) {
        s_overflowed = true;
        return false;
    }
    memcpy(s_buffer + s_length, data, len);
    s_length += len;
    s_buffer[s_length] = '\0';
    return true;
}

const char *weather_cache_data(size_t *out_len)
{
    if (out_len) {
        *out_len = s_length;
    }
    return s_buffer;
}

size_t weather_cache_capacity(void)
{
    return s_capacity;
}

bool weather_cache_is_overflowed(void)
{
    return s_overflowed;
}
