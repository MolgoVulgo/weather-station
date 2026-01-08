#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void boot_progress_init(void);
void boot_progress_set(int32_t value, const char *text);
void boot_progress_show_wifi(void);
void boot_progress_show_meteo(void);

#ifdef __cplusplus
}
#endif
