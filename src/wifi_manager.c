#include "wifi_manager.h"

#include <string.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "secrets.h"
#include "wifi_portal.h"

static const char *TAG = "WiFi";

#define WIFI_NVS_NAMESPACE "wifi_cfg"
#define WIFI_NVS_KEY_SSID "ssid"
#define WIFI_NVS_KEY_PASS "pass"

static wifi_manager_credentials_t wifi_cfg = {
    .ssid = WIFI_SSID,
    .password = WIFI_PASSWORD,
};
static bool wifi_missing_credentials = false;
static int wifi_retry_count = 0;
static const char *ap_password = "stationmeteo";

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    (void)arg;
    (void)event_data;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi STA start");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi deconnecte, reconnexion");
        if (wifi_portal_is_active()) {
            return;
        }
        wifi_retry_count++;
        if (wifi_retry_count >= 5) {
            ESP_LOGW(TAG, "Echec connexion, passage en AP");
            wifi_portal_start("StationMeteo", ap_password);
            return;
        }
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "WiFi IP: " IPSTR, IP2STR(&event->ip_info.ip));
        wifi_retry_count = 0;
    }
}

static esp_err_t wifi_manager_nvs_load(bool *out_missing)
{
    nvs_handle_t nvs = 0;
    esp_err_t ret = nvs_open(WIFI_NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed: %s", esp_err_to_name(ret));
        return ret;
    }

    bool missing = false;
    size_t ssid_len = sizeof(wifi_cfg.ssid);
    ret = nvs_get_str(nvs, WIFI_NVS_KEY_SSID, wifi_cfg.ssid, &ssid_len);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        missing = true;
        strlcpy(wifi_cfg.ssid, WIFI_SSID, sizeof(wifi_cfg.ssid));
    } else if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Lecture NVS SSID echouee: %s", esp_err_to_name(ret));
        strlcpy(wifi_cfg.ssid, WIFI_SSID, sizeof(wifi_cfg.ssid));
    }

    size_t pass_len = sizeof(wifi_cfg.password);
    ret = nvs_get_str(nvs, WIFI_NVS_KEY_PASS, wifi_cfg.password, &pass_len);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        missing = true;
        strlcpy(wifi_cfg.password, WIFI_PASSWORD, sizeof(wifi_cfg.password));
    } else if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Lecture NVS mot de passe echouee: %s", esp_err_to_name(ret));
        strlcpy(wifi_cfg.password, WIFI_PASSWORD, sizeof(wifi_cfg.password));
    }

    nvs_close(nvs);

    if (out_missing) {
        *out_missing = missing;
    }
    wifi_missing_credentials = missing;
    return ESP_OK;
}

esp_err_t wifi_manager_save_credentials(void)
{
    nvs_handle_t nvs = 0;
    esp_err_t ret = nvs_open(WIFI_NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_set_str(nvs, WIFI_NVS_KEY_SSID, wifi_cfg.ssid);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS set ssid failed: %s", esp_err_to_name(ret));
        nvs_close(nvs);
        return ret;
    }

    ret = nvs_set_str(nvs, WIFI_NVS_KEY_PASS, wifi_cfg.password);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS set pass failed: %s", esp_err_to_name(ret));
        nvs_close(nvs);
        return ret;
    }

    ret = nvs_commit(nvs);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS commit failed: %s", esp_err_to_name(ret));
    }
    nvs_close(nvs);
    return ret;
}

esp_err_t wifi_manager_set_credentials(const char *ssid, const char *password)
{
    if (!ssid || ssid[0] == '\0' || !password) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(wifi_cfg.ssid, ssid, sizeof(wifi_cfg.ssid));
    strlcpy(wifi_cfg.password, password, sizeof(wifi_cfg.password));
    return ESP_OK;
}

esp_err_t wifi_manager_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS erase requis, tentative d'effacement");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    if (WIFI_RESET_NVS) {
        ESP_LOGW(TAG, "WIFI_RESET_NVS=1, effacement credentials WiFi");
        nvs_handle_t nvs = 0;
        if (nvs_open(WIFI_NVS_NAMESPACE, NVS_READWRITE, &nvs) == ESP_OK) {
            nvs_erase_key(nvs, WIFI_NVS_KEY_SSID);
            nvs_erase_key(nvs, WIFI_NVS_KEY_PASS);
            nvs_commit(nvs);
            nvs_close(nvs);
        }
    }

    ret = esp_netif_init();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_netif_init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "event loop init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    bool missing = false;
    ret = wifi_manager_nvs_load(&missing);
    if (ret != ESP_OK) {
        return ret;
    }
    if (missing) {
        ESP_LOGI(TAG, "NVS wifi_cfg absent, portail captif requis");
        wifi_missing_credentials = true;
    }

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {0};
    strlcpy((char *)wifi_config.sta.ssid, wifi_cfg.ssid, sizeof(wifi_config.sta.ssid));
    strlcpy((char *)wifi_config.sta.password, wifi_cfg.password, sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi init avec SSID: %s", wifi_cfg.ssid);
    if (wifi_missing_credentials) {
        ESP_LOGW(TAG, "Aucun credential, activation portail captif");
        wifi_portal_start("StationMeteo", ap_password);
    }
    return ESP_OK;
}

const wifi_manager_credentials_t *wifi_manager_get_credentials(void)
{
    return &wifi_cfg;
}

bool wifi_manager_is_portal_active(void)
{
    return wifi_portal_is_active();
}
