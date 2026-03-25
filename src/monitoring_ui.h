#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t monitoring_ui_init(void);
void monitoring_ui_start(void);
void monitoring_ui_tick(void);

#ifdef __cplusplus
}
#endif
