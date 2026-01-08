#include "wifi_portal.h"

#include <string.h>
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "wifi_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "WiFiPortal";
static httpd_handle_t s_http;
static TaskHandle_t s_dns_task;
static bool s_portal_active;
static esp_netif_t *s_ap_netif;

static const char *kPortalHtml =
    "<!doctype html><html><head><meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>WiFi Setup</title>"
    "<style>body{font-family:sans-serif;padding:16px}label{display:block;margin-top:12px}</style>"
    "</head><body><h2>Configuration WiFi</h2>"
    "<label>SSID</label><select id='ssid'></select>"
    "<button onclick='scan()'>Rafraichir</button>"
    "<label>Mot de passe</label><input id='pass' type='password'/>"
    "<button onclick='save()'>Enregistrer</button>"
    "<p id='status'></p>"
    "<script>"
    "function scan(){"
    "fetch('/scan').then(r=>r.json()).then(d=>{"
    "const s=document.getElementById('ssid');"
    "s.innerHTML='';"
    "d.ssids.forEach(v=>{const o=document.createElement('option');o.value=v;o.text=v;s.appendChild(o);});"
    "});"
    "}"
    "scan();"
    "function save(){"
    "const ssid=document.getElementById('ssid').value;"
    "const pass=document.getElementById('pass').value;"
    "fetch('/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},"
    "body:'ssid='+encodeURIComponent(ssid)+'&pass='+encodeURIComponent(pass)})"
    ".then(r=>r.text()).then(t=>{document.getElementById('status').textContent=t;});"
    "}"
    "</script></body></html>";

static esp_err_t portal_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, kPortalHtml, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t portal_scan_handler(httpd_req_t *req)
{
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false,
    };
    esp_wifi_scan_start(&scan_config, true);

    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);
    httpd_resp_set_type(req, "application/json");
    if (ap_count == 0) {
        httpd_resp_send(req, "{\"ssids\":[]}", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    if (ap_count > 50) {
        ap_count = 50;
    }

    wifi_ap_record_t *records = calloc(ap_count, sizeof(*records));
    if (!records) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Out of memory");
        return ESP_FAIL;
    }
    esp_wifi_scan_get_ap_records(&ap_count, records);
    for (uint16_t i = 0; i < ap_count; i++) {
        ESP_LOGI(TAG, "Scan SSID[%u]: %s (RSSI %d)", i, records[i].ssid, records[i].rssi);
    }

    httpd_resp_send_chunk(req, "{\"ssids\":[", HTTPD_RESP_USE_STRLEN);
    for (uint16_t i = 0; i < ap_count; i++) {
        if (i > 0) {
            httpd_resp_send_chunk(req, ",", 1);
        }
        httpd_resp_send_chunk(req, "\"", 1);
        for (const char *p = (const char *)records[i].ssid; *p != '\0'; p++) {
            if (*p == '"' || *p == '\\') {
                char esc[2] = {'\\', *p};
                httpd_resp_send_chunk(req, esc, sizeof(esc));
            } else {
                httpd_resp_send_chunk(req, p, 1);
            }
        }
        httpd_resp_send_chunk(req, "\"", 1);
    }
    httpd_resp_send_chunk(req, "]}", HTTPD_RESP_USE_STRLEN);
    httpd_resp_send_chunk(req, NULL, 0);
    free(records);
    return ESP_OK;
}

static void url_decode(char *dst, const char *src, size_t len)
{
    size_t di = 0;
    for (size_t i = 0; src[i] != '\0' && di + 1 < len; i++) {
        if (src[i] == '%' && src[i + 1] && src[i + 2]) {
            char hex[3] = {src[i + 1], src[i + 2], 0};
            dst[di++] = (char)strtol(hex, NULL, 16);
            i += 2;
        } else if (src[i] == '+') {
            dst[di++] = ' ';
        } else {
            dst[di++] = src[i];
        }
    }
    dst[di] = '\0';
}

static esp_err_t portal_save_handler(httpd_req_t *req)
{
    char body[256];
    int read = httpd_req_recv(req, body, sizeof(body) - 1);
    if (read <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid body");
        return ESP_FAIL;
    }
    body[read] = '\0';

    char ssid[WIFI_MANAGER_SSID_MAX_LEN] = {0};
    char pass[WIFI_MANAGER_PASS_MAX_LEN] = {0};

    const char *ssid_ptr = strstr(body, "ssid=");
    const char *pass_ptr = strstr(body, "pass=");
    if (ssid_ptr) {
        ssid_ptr += 5;
        const char *end = strchr(ssid_ptr, '&');
        char raw[64] = {0};
        size_t n = end ? (size_t)(end - ssid_ptr) : strlen(ssid_ptr);
        if (n >= sizeof(raw)) {
            n = sizeof(raw) - 1;
        }
        memcpy(raw, ssid_ptr, n);
        raw[n] = '\0';
        url_decode(ssid, raw, sizeof(ssid));
    }
    if (pass_ptr) {
        pass_ptr += 5;
        const char *end = strchr(pass_ptr, '&');
        char raw[128] = {0};
        size_t n = end ? (size_t)(end - pass_ptr) : strlen(pass_ptr);
        if (n >= sizeof(raw)) {
            n = sizeof(raw) - 1;
        }
        memcpy(raw, pass_ptr, n);
        raw[n] = '\0';
        url_decode(pass, raw, sizeof(pass));
    }

    if (ssid[0] == '\0') {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "SSID missing");
        return ESP_FAIL;
    }

    wifi_manager_set_credentials(ssid, pass);
    wifi_manager_save_credentials();
    httpd_resp_sendstr(req, "Sauvegarde OK. Redemarrage...");
    vTaskDelay(pdMS_TO_TICKS(500));
    esp_restart();
    return ESP_OK;
}

static esp_err_t portal_captive_handler(httpd_req_t *req)
{
    return portal_get_handler(req);
}

static esp_err_t portal_redirect_handler(httpd_req_t *req)
{
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static void dns_task(void *arg)
{
    (void)arg;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        vTaskDelete(NULL);
        return;
    }
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(53),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    uint8_t rx[256];
    while (1) {
        struct sockaddr_in from;
        socklen_t from_len = sizeof(from);
        int len = recvfrom(sock, rx, sizeof(rx), 0, (struct sockaddr *)&from, &from_len);
        if (len < 12) {
            continue;
        }
        uint8_t tx[256];
        memcpy(tx, rx, (size_t)len);
        tx[2] |= 0x80;
        tx[3] |= 0x80;
        tx[7] = 1;

        uint8_t *p = tx + len;
        *p++ = 0xC0;
        *p++ = 0x0C;
        *p++ = 0x00;
        *p++ = 0x01;
        *p++ = 0x00;
        *p++ = 0x01;
        *p++ = 0x00;
        *p++ = 0x00;
        *p++ = 0x00;
        *p++ = 0x3C;
        *p++ = 0x00;
        *p++ = 0x04;
        uint32_t ip = htonl(0xC0A80401);
        memcpy(p, &ip, 4);
        p += 4;

        sendto(sock, tx, p - tx, 0, (struct sockaddr *)&from, from_len);
    }
}

static esp_err_t portal_start_http(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    if (httpd_start(&s_http, &config) != ESP_OK) {
        return ESP_FAIL;
    }
    httpd_uri_t root = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = portal_get_handler,
        .user_ctx = NULL,
    };
    httpd_uri_t scan = {
        .uri = "/scan",
        .method = HTTP_GET,
        .handler = portal_scan_handler,
        .user_ctx = NULL,
    };
    httpd_uri_t save = {
        .uri = "/save",
        .method = HTTP_POST,
        .handler = portal_save_handler,
        .user_ctx = NULL,
    };
    httpd_uri_t captive = root;
    captive.uri = "/generate_204";
    captive.handler = portal_captive_handler;
    httpd_uri_t captive_short = captive;
    captive_short.uri = "/gen_204";
    httpd_uri_t captive_ios = captive;
    captive_ios.uri = "/hotspot-detect.html";
    httpd_uri_t captive_win = captive;
    captive_win.uri = "/ncsi.txt";
    httpd_uri_t captive_ms = captive;
    captive_ms.uri = "/connecttest.txt";
    httpd_uri_t wildcard = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = portal_redirect_handler,
        .user_ctx = NULL,
    };
    httpd_register_uri_handler(s_http, &root);
    httpd_register_uri_handler(s_http, &scan);
    httpd_register_uri_handler(s_http, &save);
    httpd_register_uri_handler(s_http, &captive);
    httpd_register_uri_handler(s_http, &captive_short);
    httpd_register_uri_handler(s_http, &captive_ios);
    httpd_register_uri_handler(s_http, &captive_win);
    httpd_register_uri_handler(s_http, &captive_ms);
    httpd_register_uri_handler(s_http, &wildcard);
    return ESP_OK;
}

esp_err_t wifi_portal_start(const char *ssid, const char *password)
{
    if (s_portal_active) {
        return ESP_OK;
    }
    if (!ssid || !password) {
        return ESP_ERR_INVALID_ARG;
    }
    s_portal_active = true;

    s_ap_netif = esp_netif_create_default_wifi_ap();
    if (s_ap_netif) {
        esp_netif_ip_info_t ip_info = {0};
        ip_info.ip.addr = ipaddr_addr("192.168.4.1");
        ip_info.gw.addr = ipaddr_addr("192.168.4.1");
        ip_info.netmask.addr = ipaddr_addr("255.255.255.0");
        esp_netif_dhcps_stop(s_ap_netif);
        esp_netif_set_ip_info(s_ap_netif, &ip_info);
        esp_netif_dhcps_start(s_ap_netif);
    }

    wifi_config_t wifi_config = {0};
    strlcpy((char *)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid));
    strlcpy((char *)wifi_config.ap.password, password, sizeof(wifi_config.ap.password));
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.ssid_hidden = 0;

    esp_wifi_set_mode(WIFI_MODE_APSTA);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "AP SSID: %s", ssid);
    ESP_LOGI(TAG, "AP PASS: %s", password);
    ESP_LOGI(TAG, "Portail: http://192.168.4.1/");

    if (portal_start_http() != ESP_OK) {
        ESP_LOGE(TAG, "HTTP server start failed");
    }
    xTaskCreate(dns_task, "dns_task", 4096, NULL, 3, &s_dns_task);
    return ESP_OK;
}

bool wifi_portal_is_active(void)
{
    return s_portal_active;
}
