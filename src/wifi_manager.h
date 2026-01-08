#pragma once

#include <stdbool.h>
#include "esp_err.h"

#define WIFI_MANAGER_SSID_MAX_LEN 32
#define WIFI_MANAGER_PASS_MAX_LEN 64

typedef struct {
    char ssid[WIFI_MANAGER_SSID_MAX_LEN];
    char password[WIFI_MANAGER_PASS_MAX_LEN];
} wifi_manager_credentials_t;

esp_err_t wifi_manager_init(void);
const wifi_manager_credentials_t *wifi_manager_get_credentials(void);
bool wifi_manager_is_portal_active(void);

/* Pour future configuration Web/écran. */
esp_err_t wifi_manager_set_credentials(const char *ssid, const char *password);
esp_err_t wifi_manager_save_credentials(void);
