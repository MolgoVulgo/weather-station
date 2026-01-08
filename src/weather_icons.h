#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t weather_icons_set_main(uint16_t code, uint8_t variant);

#ifdef __cplusplus
}
#endif
