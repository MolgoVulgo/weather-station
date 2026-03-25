#include "monitoring_service.h"

#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <string>

#include "cJSON.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "secrets.h"

#ifndef MONITOR_API_DASHBOARD_PATH
#define MONITOR_API_DASHBOARD_PATH "/api/v1/dashboard"
#endif

#ifndef MONITOR_API_HISTORY_PATH
#define MONITOR_API_HISTORY_PATH "/api/v1/history"
#endif

#ifndef MONITOR_API_HISTORY_WINDOW_S
#define MONITOR_API_HISTORY_WINDOW_S 10
#endif

#ifndef MONITOR_API_HISTORY_STEP_S
#define MONITOR_API_HISTORY_STEP_S 1
#endif

#ifndef MONITOR_POLL_DASHBOARD_MS
#define MONITOR_POLL_DASHBOARD_MS 500
#endif

#ifndef MONITOR_POLL_HISTORY_MS
#define MONITOR_POLL_HISTORY_MS 500
#endif

static const char *TAG = "MonitoringService";

static TaskHandle_t s_task = NULL;
static SemaphoreHandle_t s_snapshot_mutex = NULL;
static esp_timer_handle_t s_timer = NULL;
static bool s_timer_running = false;
static bool s_started = false;
static bool s_active = false;
static int64_t s_last_dashboard_ms = 0;
static int64_t s_last_history_ms = 0;
static monitoring_snapshot_t s_snapshot = {};

static void copy_text(char *dst, size_t dst_len, const char *src)
{
    if (!dst || dst_len == 0) {
        return;
    }
    if (!src) {
        dst[0] = '\0';
        return;
    }
    snprintf(dst, dst_len, "%s", src);
    dst[dst_len - 1] = '\0';
}

static bool snapshot_lock(TickType_t timeout_ticks)
{
    return s_snapshot_mutex &&
           xSemaphoreTake(s_snapshot_mutex, timeout_ticks) == pdTRUE;
}

static void snapshot_unlock(void)
{
    if (s_snapshot_mutex) {
        xSemaphoreGive(s_snapshot_mutex);
    }
}

static void snapshot_set_error(const char *message)
{
    if (!snapshot_lock(pdMS_TO_TICKS(50))) {
        return;
    }
    copy_text(s_snapshot.last_error, sizeof(s_snapshot.last_error), message);
    s_snapshot.sequence++;
    snapshot_unlock();
}

static void snapshot_set_errorf(const char *fmt, const char *arg)
{
    if (!snapshot_lock(pdMS_TO_TICKS(50))) {
        return;
    }
    int written = snprintf(s_snapshot.last_error,
                           sizeof(s_snapshot.last_error),
                           fmt ? fmt : "error",
                           arg ? arg : "");
    if (written < 0) {
        s_snapshot.last_error[0] = '\0';
    } else {
        s_snapshot.last_error[sizeof(s_snapshot.last_error) - 1] = '\0';
    }
    s_snapshot.sequence++;
    snapshot_unlock();
}

static void snapshot_clear_error(void)
{
    if (!snapshot_lock(pdMS_TO_TICKS(50))) {
        return;
    }
    if (s_snapshot.last_error[0] != '\0') {
        s_snapshot.last_error[0] = '\0';
        s_snapshot.sequence++;
    }
    snapshot_unlock();
}

static bool parse_metric_value(cJSON *obj, const char *key, monitoring_metric_t *out)
{
    if (!obj || !key || !out) {
        return false;
    }

    out->valid = false;
    out->value = 0.0f;

    cJSON *metric = cJSON_GetObjectItemCaseSensitive(obj, key);
    if (!cJSON_IsObject(metric)) {
        return false;
    }

    cJSON *valid = cJSON_GetObjectItemCaseSensitive(metric, "valid");
    if (cJSON_IsBool(valid) && !cJSON_IsTrue(valid)) {
        return false;
    }

    cJSON *display = cJSON_GetObjectItemCaseSensitive(metric, "value_display");
    if (!cJSON_IsNumber(display)) {
        return false;
    }

    out->valid = true;
    out->value = (float)display->valuedouble;
    return true;
}

static bool parse_group_metric(cJSON *root,
                               const char *group_key,
                               const char *metric_key,
                               monitoring_metric_t *out)
{
    if (!root || !group_key || !metric_key || !out) {
        return false;
    }
    cJSON *group = cJSON_GetObjectItemCaseSensitive(root, group_key);
    if (!cJSON_IsObject(group)) {
        out->valid = false;
        out->value = 0.0f;
        return false;
    }
    return parse_metric_value(group, metric_key, out);
}

static size_t parse_history_array(cJSON *series,
                                  const char *key,
                                  float *out_values,
                                  bool *out_valid,
                                  size_t max_points)
{
    if (!series || !key || !out_values || !out_valid || max_points == 0) {
        return 0;
    }

    cJSON *arr = cJSON_GetObjectItemCaseSensitive(series, key);
    if (!cJSON_IsArray(arr)) {
        return 0;
    }

    size_t count = 0;
    int array_size = cJSON_GetArraySize(arr);
    for (int i = 0; i < array_size && count < max_points; ++i) {
        cJSON *item = cJSON_GetArrayItem(arr, i);
        if (cJSON_IsNumber(item)) {
            out_values[count] = (float)item->valuedouble;
            out_valid[count] = true;
        } else {
            out_values[count] = 0.0f;
            out_valid[count] = false;
        }
        count++;
    }
    return count;
}

static esp_err_t http_get_json(const char *url, std::string &out)
{
    out.clear();
    if (!url || !*url) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_http_client_config_t config = {};
    config.url = url;
    config.timeout_ms = 2500;
    config.keep_alive_enable = false;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        return ESP_FAIL;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        esp_http_client_cleanup(client);
        return err;
    }

    int headers = esp_http_client_fetch_headers(client);
    if (headers < 0) {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    int code = esp_http_client_get_status_code(client);
    if (code != 200) {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    char buffer[512];
    int read_len = 0;
    while ((read_len = esp_http_client_read(client, buffer, sizeof(buffer))) > 0) {
        out.append(buffer, (size_t)read_len);
        if (out.size() > 32768) {
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            return ESP_ERR_NO_MEM;
        }
    }

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    if (read_len < 0 || out.empty()) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

static bool parse_dashboard_json(const char *json, monitoring_snapshot_t *dst)
{
    if (!json || !dst) {
        return false;
    }

    cJSON *root = cJSON_Parse(json);
    if (!root) {
        return false;
    }

    cJSON *host = cJSON_GetObjectItemCaseSensitive(root, "host");
    if (cJSON_IsString(host) && host->valuestring) {
        copy_text(dst->host, sizeof(dst->host), host->valuestring);
    } else {
        dst->host[0] = '\0';
    }

    cJSON *ts = cJSON_GetObjectItemCaseSensitive(root, "ts");
    if (cJSON_IsNumber(ts)) {
        dst->dashboard_ts = (int32_t)ts->valuedouble;
    }

    cJSON *state = cJSON_GetObjectItemCaseSensitive(root, "state");
    if (cJSON_IsObject(state)) {
        cJSON *stale_ms = cJSON_GetObjectItemCaseSensitive(state, "stale_ms");
        if (cJSON_IsNumber(stale_ms)) {
            dst->stale_ms = (int32_t)stale_ms->valuedouble;
        }
        cJSON *ok = cJSON_GetObjectItemCaseSensitive(state, "ok");
        dst->state_ok = cJSON_IsTrue(ok);
    } else {
        dst->stale_ms = 0;
        dst->state_ok = false;
    }

    parse_group_metric(root, "cpu", "pct", &dst->cpu_pct);
    parse_group_metric(root, "cpu", "temp_c", &dst->cpu_temp_c);
    parse_group_metric(root, "mem", "pct", &dst->mem_pct);
    parse_group_metric(root, "mem", "used_b", &dst->mem_used_b);
    parse_group_metric(root, "mem", "total_b", &dst->mem_total_b);
    parse_group_metric(root, "gpu", "pct", &dst->gpu_pct);
    parse_group_metric(root, "gpu", "temp_c", &dst->gpu_temp_c);
    parse_group_metric(root, "gpu", "power_w", &dst->gpu_power_w);

    cJSON_Delete(root);
    dst->dashboard_valid = true;
    return true;
}

static bool parse_history_json(const char *json, monitoring_snapshot_t *dst)
{
    if (!json || !dst) {
        return false;
    }

    cJSON *root = cJSON_Parse(json);
    if (!root) {
        return false;
    }

    cJSON *ts = cJSON_GetObjectItemCaseSensitive(root, "ts");
    if (cJSON_IsNumber(ts)) {
        dst->history_ts = (int32_t)ts->valuedouble;
    }

    cJSON *window_s = cJSON_GetObjectItemCaseSensitive(root, "window_s");
    if (cJSON_IsNumber(window_s)) {
        dst->window_s = (int32_t)window_s->valuedouble;
    }

    cJSON *step_s = cJSON_GetObjectItemCaseSensitive(root, "step_s");
    if (cJSON_IsNumber(step_s)) {
        dst->step_s = (int32_t)step_s->valuedouble;
    }

    memset(dst->history_cpu_pct_valid, 0, sizeof(dst->history_cpu_pct_valid));
    memset(dst->history_gpu_pct_valid, 0, sizeof(dst->history_gpu_pct_valid));
    memset(dst->history_cpu_temp_c_valid, 0, sizeof(dst->history_cpu_temp_c_valid));
    memset(dst->history_gpu_temp_c_valid, 0, sizeof(dst->history_gpu_temp_c_valid));

    cJSON *series = cJSON_GetObjectItemCaseSensitive(root, "series");
    size_t cpu_pct_count = parse_history_array(series,
                                               "cpu_pct",
                                               dst->history_cpu_pct,
                                               dst->history_cpu_pct_valid,
                                               MONITORING_HISTORY_POINTS_MAX);
    size_t gpu_pct_count = parse_history_array(series,
                                               "gpu_pct",
                                               dst->history_gpu_pct,
                                               dst->history_gpu_pct_valid,
                                               MONITORING_HISTORY_POINTS_MAX);
    size_t cpu_temp_count = parse_history_array(series,
                                                "cpu_temp_c",
                                                dst->history_cpu_temp_c,
                                                dst->history_cpu_temp_c_valid,
                                                MONITORING_HISTORY_POINTS_MAX);
    size_t gpu_temp_count = parse_history_array(series,
                                                "gpu_temp_c",
                                                dst->history_gpu_temp_c,
                                                dst->history_gpu_temp_c_valid,
                                                MONITORING_HISTORY_POINTS_MAX);

    size_t max_count = std::max(std::max(cpu_pct_count, gpu_pct_count),
                                std::max(cpu_temp_count, gpu_temp_count));
    dst->history_count = max_count;
    dst->history_valid = max_count > 0;

    cJSON_Delete(root);
    return dst->history_valid;
}

static bool fetch_dashboard_once(void)
{
    char url[256];
    int written = snprintf(url,
                           sizeof(url),
                           "http://%s:%d%s",
                           MONITOR_API_HOST,
                           MONITOR_API_PORT,
                           MONITOR_API_DASHBOARD_PATH);
    if (written <= 0 || (size_t)written >= sizeof(url)) {
        snapshot_set_error("dashboard url overflow");
        return false;
    }

    std::string payload;
    esp_err_t err = http_get_json(url, payload);
    if (err != ESP_OK) {
        snapshot_set_errorf("dashboard http: %s", esp_err_to_name(err));
        return false;
    }

    monitoring_snapshot_t parsed = {};
    if (!parse_dashboard_json(payload.c_str(), &parsed)) {
        snapshot_set_error("dashboard parse error");
        return false;
    }

    if (!snapshot_lock(pdMS_TO_TICKS(100))) {
        return false;
    }
    s_snapshot.dashboard_valid = parsed.dashboard_valid;
    s_snapshot.dashboard_ts = parsed.dashboard_ts;
    s_snapshot.stale_ms = parsed.stale_ms;
    s_snapshot.state_ok = parsed.state_ok;
    s_snapshot.cpu_pct = parsed.cpu_pct;
    s_snapshot.cpu_temp_c = parsed.cpu_temp_c;
    s_snapshot.mem_pct = parsed.mem_pct;
    s_snapshot.mem_used_b = parsed.mem_used_b;
    s_snapshot.mem_total_b = parsed.mem_total_b;
    s_snapshot.gpu_pct = parsed.gpu_pct;
    s_snapshot.gpu_temp_c = parsed.gpu_temp_c;
    s_snapshot.gpu_power_w = parsed.gpu_power_w;
    copy_text(s_snapshot.host, sizeof(s_snapshot.host), parsed.host);
    s_snapshot.sequence++;
    snapshot_unlock();
    return true;
}

static bool fetch_history_once(void)
{
    char url[256];
    int written = snprintf(url,
                           sizeof(url),
                           "http://%s:%d%s?window=%d&step=%d",
                           MONITOR_API_HOST,
                           MONITOR_API_PORT,
                           MONITOR_API_HISTORY_PATH,
                           MONITOR_API_HISTORY_WINDOW_S,
                           MONITOR_API_HISTORY_STEP_S);
    if (written <= 0 || (size_t)written >= sizeof(url)) {
        snapshot_set_error("history url overflow");
        return false;
    }

    std::string payload;
    esp_err_t err = http_get_json(url, payload);
    if (err != ESP_OK) {
        snapshot_set_errorf("history http: %s", esp_err_to_name(err));
        return false;
    }

    monitoring_snapshot_t parsed = {};
    if (!parse_history_json(payload.c_str(), &parsed)) {
        snapshot_set_error("history parse error");
        return false;
    }

    if (!snapshot_lock(pdMS_TO_TICKS(100))) {
        return false;
    }
    s_snapshot.history_valid = parsed.history_valid;
    s_snapshot.history_ts = parsed.history_ts;
    s_snapshot.window_s = parsed.window_s;
    s_snapshot.step_s = parsed.step_s;
    s_snapshot.history_count = parsed.history_count;
    memcpy(s_snapshot.history_cpu_pct, parsed.history_cpu_pct, sizeof(s_snapshot.history_cpu_pct));
    memcpy(s_snapshot.history_cpu_pct_valid, parsed.history_cpu_pct_valid, sizeof(s_snapshot.history_cpu_pct_valid));
    memcpy(s_snapshot.history_gpu_pct, parsed.history_gpu_pct, sizeof(s_snapshot.history_gpu_pct));
    memcpy(s_snapshot.history_gpu_pct_valid, parsed.history_gpu_pct_valid, sizeof(s_snapshot.history_gpu_pct_valid));
    memcpy(s_snapshot.history_cpu_temp_c, parsed.history_cpu_temp_c, sizeof(s_snapshot.history_cpu_temp_c));
    memcpy(s_snapshot.history_cpu_temp_c_valid, parsed.history_cpu_temp_c_valid, sizeof(s_snapshot.history_cpu_temp_c_valid));
    memcpy(s_snapshot.history_gpu_temp_c, parsed.history_gpu_temp_c, sizeof(s_snapshot.history_gpu_temp_c));
    memcpy(s_snapshot.history_gpu_temp_c_valid, parsed.history_gpu_temp_c_valid, sizeof(s_snapshot.history_gpu_temp_c_valid));
    s_snapshot.sequence++;
    snapshot_unlock();
    return true;
}

static int64_t monitor_period_us(void)
{
    int32_t dash_ms = MONITOR_POLL_DASHBOARD_MS;
    int32_t history_ms = MONITOR_POLL_HISTORY_MS;
    int32_t period_ms = std::min(dash_ms, history_ms);
    if (period_ms < 100) {
        period_ms = 100;
    }
    return (int64_t)period_ms * 1000LL;
}

static void monitor_timer_cb(void *arg)
{
    (void)arg;
    if (s_task) {
        xTaskNotifyGive(s_task);
    }
}

static void monitor_task(void *arg)
{
    (void)arg;
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (!s_active) {
            continue;
        }

        int64_t now_ms = esp_timer_get_time() / 1000;
        bool due_dashboard = (s_last_dashboard_ms == 0) ||
                             ((now_ms - s_last_dashboard_ms) >= MONITOR_POLL_DASHBOARD_MS);
        bool due_history = (s_last_history_ms == 0) ||
                           ((now_ms - s_last_history_ms) >= MONITOR_POLL_HISTORY_MS);

        bool ok_dashboard = true;
        bool ok_history = true;

        if (due_dashboard) {
            ok_dashboard = fetch_dashboard_once();
            if (ok_dashboard) {
                s_last_dashboard_ms = now_ms;
            }
        }

        if (due_history) {
            ok_history = fetch_history_once();
            if (ok_history) {
                s_last_history_ms = now_ms;
            }
        }

        if ((due_dashboard || due_history) && ok_dashboard && ok_history) {
            snapshot_clear_error();
        }
    }
}

extern "C" esp_err_t monitoring_service_start(void)
{
    if (s_started) {
        return ESP_OK;
    }

    s_snapshot_mutex = xSemaphoreCreateMutex();
    if (!s_snapshot_mutex) {
        return ESP_ERR_NO_MEM;
    }

    s_snapshot.window_s = MONITOR_API_HISTORY_WINDOW_S;
    s_snapshot.step_s = MONITOR_API_HISTORY_STEP_S;

    BaseType_t task_ok = xTaskCreate(
        monitor_task,
        "MonitoringTask",
        8192,
        NULL,
        tskIDLE_PRIORITY + 2,
        &s_task);
    if (task_ok != pdPASS) {
        vSemaphoreDelete(s_snapshot_mutex);
        s_snapshot_mutex = NULL;
        s_task = NULL;
        return ESP_FAIL;
    }

    esp_timer_create_args_t timer_args = {
        .callback = &monitor_timer_cb,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "monitor_timer",
        .skip_unhandled_events = true,
    };
    esp_err_t err = esp_timer_create(&timer_args, &s_timer);
    if (err != ESP_OK) {
        s_timer = NULL;
        if (s_task) {
            vTaskDelete(s_task);
            s_task = NULL;
        }
        if (s_snapshot_mutex) {
            vSemaphoreDelete(s_snapshot_mutex);
            s_snapshot_mutex = NULL;
        }
        return err;
    }

    s_started = true;
    ESP_LOGI(TAG, "Service demarre (%s:%d)", MONITOR_API_HOST, MONITOR_API_PORT);
    return ESP_OK;
}

extern "C" esp_err_t monitoring_service_set_active(bool active)
{
    if (!s_started || !s_timer) {
        return ESP_ERR_INVALID_STATE;
    }
    if (s_active == active) {
        return ESP_OK;
    }

    s_active = active;
    if (active) {
        s_last_dashboard_ms = 0;
        s_last_history_ms = 0;
        if (s_timer_running) {
            esp_timer_stop(s_timer);
            s_timer_running = false;
        }
        esp_err_t err = esp_timer_start_periodic(s_timer, monitor_period_us());
        if (err != ESP_OK) {
            s_active = false;
            ESP_LOGE(TAG, "Timer start failed: %s", esp_err_to_name(err));
            return err;
        }
        s_timer_running = true;
        if (s_task) {
            xTaskNotifyGive(s_task);
        }
        ESP_LOGI(TAG, "Collecte active");
    } else {
        if (s_timer_running) {
            esp_timer_stop(s_timer);
            s_timer_running = false;
        }
        ESP_LOGI(TAG, "Collecte suspendue");
    }
    return ESP_OK;
}

extern "C" bool monitoring_service_is_active(void)
{
    return s_active;
}

extern "C" bool monitoring_service_get_snapshot(monitoring_snapshot_t *out)
{
    if (!out || !s_snapshot_mutex) {
        return false;
    }
    if (!snapshot_lock(pdMS_TO_TICKS(50))) {
        return false;
    }
    *out = s_snapshot;
    snapshot_unlock();
    return out->dashboard_valid || out->history_valid || out->last_error[0] != '\0';
}
