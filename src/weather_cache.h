#pragma once

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool weather_cache_init(size_t max_size);
void weather_cache_reset(void);
bool weather_cache_write(const char *data, size_t len);
const char *weather_cache_data(size_t *out_len);
size_t weather_cache_capacity(void);
bool weather_cache_is_overflowed(void);

#ifdef __cplusplus
}
#endif
