#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MONITORING_HISTORY_POINTS_MAX 64

typedef struct {
    bool valid;
    float value;
} monitoring_metric_t;

typedef struct {
    uint32_t sequence;
    bool dashboard_valid;
    bool history_valid;
    int32_t dashboard_ts;
    int32_t history_ts;
    int32_t stale_ms;
    bool state_ok;
    int32_t window_s;
    int32_t step_s;
    char host[64];
    char last_error[96];

    monitoring_metric_t cpu_pct;
    monitoring_metric_t cpu_temp_c;
    monitoring_metric_t mem_pct;
    monitoring_metric_t mem_used_b;
    monitoring_metric_t mem_total_b;
    monitoring_metric_t gpu_pct;
    monitoring_metric_t gpu_temp_c;
    monitoring_metric_t gpu_power_w;

    size_t history_count;
    float history_cpu_pct[MONITORING_HISTORY_POINTS_MAX];
    bool history_cpu_pct_valid[MONITORING_HISTORY_POINTS_MAX];
    float history_gpu_pct[MONITORING_HISTORY_POINTS_MAX];
    bool history_gpu_pct_valid[MONITORING_HISTORY_POINTS_MAX];
    float history_cpu_temp_c[MONITORING_HISTORY_POINTS_MAX];
    bool history_cpu_temp_c_valid[MONITORING_HISTORY_POINTS_MAX];
    float history_gpu_temp_c[MONITORING_HISTORY_POINTS_MAX];
    bool history_gpu_temp_c_valid[MONITORING_HISTORY_POINTS_MAX];
} monitoring_snapshot_t;

esp_err_t monitoring_service_start(void);
esp_err_t monitoring_service_set_active(bool active);
bool monitoring_service_is_active(void);
bool monitoring_service_get_snapshot(monitoring_snapshot_t *out);

#ifdef __cplusplus
}
#endif
