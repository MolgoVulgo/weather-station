#pragma once

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void temp_unit_init(void);
bool temp_unit_is_fahrenheit(void);
void temp_unit_set_fahrenheit(bool is_fahrenheit);

void temp_unit_set_last_c(float temp_c);
bool temp_unit_has_last(void);
float temp_unit_get_last_c(void);

void temp_unit_format(float temp_c, char *out, size_t out_len);
void temp_unit_format_range(float temp_min_c, float temp_max_c, char *out, size_t out_len);

#ifdef __cplusplus
}
#endif
