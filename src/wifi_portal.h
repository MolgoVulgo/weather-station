#pragma once

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t wifi_portal_start(const char *ssid, const char *password);
esp_err_t wifi_portal_start_sta(void);
bool wifi_portal_is_active(void);

#ifdef __cplusplus
}
#endif
