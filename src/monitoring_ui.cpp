#include "monitoring_ui.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "lvgl.h"
#include "monitoring_service.h"
#include "ui/actions.h"
#include "ui/fonts.h"
#include "ui/screens.h"

static const char *TAG = "MonitoringUI";
#define MONITORING_Y_SCALE_LABEL_COUNT 5

typedef struct {
    lv_obj_t *host_meta;
    lv_obj_t *status;

    lv_obj_t *cpu_pct;
    lv_obj_t *cpu_temp;
    lv_obj_t *mem_pct;
    lv_obj_t *mem_used;
    lv_obj_t *mem_total;
    lv_obj_t *gpu_pct;
    lv_obj_t *gpu_temp;
    lv_obj_t *gpu_power;

    lv_obj_t *usage_chart;
    lv_obj_t *temp_chart;
    lv_chart_series_t *usage_cpu_series;
    lv_chart_series_t *usage_gpu_series;
    lv_chart_series_t *temp_cpu_series;
    lv_chart_series_t *temp_gpu_series;
    lv_obj_t *usage_y_labels[MONITORING_Y_SCALE_LABEL_COUNT];
    lv_obj_t *temp_y_labels[MONITORING_Y_SCALE_LABEL_COUNT];
} monitoring_ui_state_t;

static monitoring_ui_state_t s_ui = {};
static lv_timer_t *s_ui_timer = NULL;
static bool s_ui_ready = false;
static bool s_last_active = false;
static bool s_force_refresh = true;
static uint32_t s_last_sequence = 0;

static lv_coord_t s_usage_cpu_points[MONITORING_HISTORY_POINTS_MAX];
static lv_coord_t s_usage_gpu_points[MONITORING_HISTORY_POINTS_MAX];
static lv_coord_t s_temp_cpu_points[MONITORING_HISTORY_POINTS_MAX];
static lv_coord_t s_temp_gpu_points[MONITORING_HISTORY_POINTS_MAX];

static void set_text(lv_obj_t *label, const char *text)
{
    if (!label) {
        return;
    }
    lv_label_set_text(label, text ? text : "");
}

static lv_obj_t *create_panel(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_pos(panel, x, y);
    lv_obj_set_size(panel, w, h);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x171a21), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x2a3140), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(panel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(panel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(panel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(panel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(panel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    return panel;
}

static lv_obj_t *create_value_label(lv_obj_t *parent, lv_coord_t x, lv_coord_t y)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_font(label, &ui_font_ui_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_color_hex(0xd8deea), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(label, "null");
    return label;
}

static lv_obj_t *create_muted_label(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, const char *text)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_color(label, lv_color_hex(0x90a0bc), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(label, text ? text : "");
    return label;
}

static lv_obj_t *create_axis_value_label(lv_obj_t *parent)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_color_hex(0x90a0bc), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(label, "-");
    return label;
}

static void update_chart_y_scale_labels(lv_obj_t *chart,
                                        lv_obj_t *labels[MONITORING_Y_SCALE_LABEL_COUNT],
                                        lv_coord_t range_min,
                                        lv_coord_t range_max)
{
    if (!chart) {
        return;
    }

    if (range_max < range_min) {
        lv_coord_t tmp = range_min;
        range_min = range_max;
        range_max = tmp;
    }

    lv_coord_t chart_x = lv_obj_get_x(chart);
    lv_coord_t chart_y = lv_obj_get_y(chart);
    lv_coord_t chart_h = lv_obj_get_height(chart);
    if (chart_h < 1) {
        chart_h = 1;
    }

    lv_coord_t label_w = (chart_x > 4) ? (chart_x - 4) : 10;
    char text[16];
    float span = (float)(range_max - range_min);

    for (size_t i = 0; i < MONITORING_Y_SCALE_LABEL_COUNT; ++i) {
        lv_obj_t *label = labels[i];
        if (!label) {
            continue;
        }

        float ratio = (MONITORING_Y_SCALE_LABEL_COUNT > 1)
                          ? ((float)i / (float)(MONITORING_Y_SCALE_LABEL_COUNT - 1))
                          : 0.0f;
        float value = (float)range_max - (span * ratio);
        snprintf(text, sizeof(text), "%ld", lroundf(value));
        set_text(label, text);

        lv_obj_set_width(label, label_w);
        lv_obj_update_layout(label);
        lv_coord_t label_h = lv_obj_get_height(label);
        lv_coord_t y = chart_y + (lv_coord_t)lroundf((double)(chart_h - 1) * (double)ratio) - (label_h / 2);
        lv_obj_set_pos(label, 0, y);
    }
}

static void format_metric_percent(char *out, size_t out_len, monitoring_metric_t metric)
{
    if (!out || out_len == 0) {
        return;
    }
    if (!metric.valid) {
        snprintf(out, out_len, "null");
        return;
    }
    snprintf(out, out_len, "%.1f%%", (double)metric.value);
}

static void format_metric_temp(char *out, size_t out_len, monitoring_metric_t metric)
{
    if (!out || out_len == 0) {
        return;
    }
    if (!metric.valid) {
        snprintf(out, out_len, "null");
        return;
    }
    snprintf(out, out_len, "%.1f C", (double)metric.value);
}

static void format_metric_power(char *out, size_t out_len, monitoring_metric_t metric)
{
    if (!out || out_len == 0) {
        return;
    }
    if (!metric.valid) {
        snprintf(out, out_len, "null");
        return;
    }
    snprintf(out, out_len, "%.1f W", (double)metric.value);
}

static void format_metric_bytes_gib(char *out, size_t out_len, monitoring_metric_t metric)
{
    if (!out || out_len == 0) {
        return;
    }
    if (!metric.valid) {
        snprintf(out, out_len, "null");
        return;
    }
    double gib = (double)metric.value / (1024.0 * 1024.0 * 1024.0);
    snprintf(out, out_len, "%.1f GiB", gib);
}

static void chart_fill_points(lv_coord_t *dst,
                              const float *values,
                              const bool *valid,
                              size_t count)
{
    if (!dst || !values || !valid) {
        return;
    }
    for (size_t i = 0; i < MONITORING_HISTORY_POINTS_MAX; ++i) {
        if (i < count && valid[i]) {
            dst[i] = (lv_coord_t)lroundf(values[i]);
        } else {
            dst[i] = LV_CHART_POINT_NONE;
        }
    }
}

static void charts_apply_history(const monitoring_snapshot_t *snap)
{
    if (!snap || !s_ui.usage_chart || !s_ui.temp_chart) {
        return;
    }

    size_t count = snap->history_count;
    if (count > MONITORING_HISTORY_POINTS_MAX) {
        count = MONITORING_HISTORY_POINTS_MAX;
    }
    if (count < 2) {
        count = 2;
    }

    chart_fill_points(s_usage_cpu_points, snap->history_cpu_pct, snap->history_cpu_pct_valid, count);
    chart_fill_points(s_usage_gpu_points, snap->history_gpu_pct, snap->history_gpu_pct_valid, count);
    chart_fill_points(s_temp_cpu_points, snap->history_cpu_temp_c, snap->history_cpu_temp_c_valid, count);
    chart_fill_points(s_temp_gpu_points, snap->history_gpu_temp_c, snap->history_gpu_temp_c_valid, count);

    lv_chart_set_range(s_ui.usage_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_point_count(s_ui.usage_chart, (uint16_t)count);
    lv_chart_refresh(s_ui.usage_chart);
    update_chart_y_scale_labels(s_ui.usage_chart, s_ui.usage_y_labels, 0, 100);

    bool found_temp = false;
    float min_temp = 0.0f;
    float max_temp = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        if (snap->history_cpu_temp_c_valid[i]) {
            float v = snap->history_cpu_temp_c[i];
            if (!found_temp) {
                min_temp = v;
                max_temp = v;
                found_temp = true;
            } else {
                if (v < min_temp) {
                    min_temp = v;
                }
                if (v > max_temp) {
                    max_temp = v;
                }
            }
        }
        if (snap->history_gpu_temp_c_valid[i]) {
            float v = snap->history_gpu_temp_c[i];
            if (!found_temp) {
                min_temp = v;
                max_temp = v;
                found_temp = true;
            } else {
                if (v < min_temp) {
                    min_temp = v;
                }
                if (v > max_temp) {
                    max_temp = v;
                }
            }
        }
    }

    lv_coord_t range_min = 20;
    lv_coord_t range_max = 90;
    if (found_temp) {
        float low = floorf(min_temp - 2.0f);
        float high = ceilf(max_temp + 2.0f);
        if ((high - low) < 10.0f) {
            float center = (high + low) * 0.5f;
            low = floorf(center - 5.0f);
            high = ceilf(center + 5.0f);
        }
        range_min = (lv_coord_t)low;
        range_max = (lv_coord_t)high;
    }

    lv_chart_set_range(s_ui.temp_chart, LV_CHART_AXIS_PRIMARY_Y, range_min, range_max);
    lv_chart_set_point_count(s_ui.temp_chart, (uint16_t)count);
    lv_chart_refresh(s_ui.temp_chart);
    update_chart_y_scale_labels(s_ui.temp_chart, s_ui.temp_y_labels, range_min, range_max);
}

static void apply_snapshot(const monitoring_snapshot_t *snap)
{
    if (!snap) {
        return;
    }

    char text[128];

    snprintf(text,
             sizeof(text),
             "host: %.*s",
             (int)sizeof(snap->host) - 1,
             snap->host[0] ? snap->host : "-");
    set_text(s_ui.host_meta, text);

    format_metric_percent(text, sizeof(text), snap->cpu_pct);
    set_text(s_ui.cpu_pct, text);
    format_metric_temp(text, sizeof(text), snap->cpu_temp_c);
    set_text(s_ui.cpu_temp, text);

    format_metric_percent(text, sizeof(text), snap->mem_pct);
    set_text(s_ui.mem_pct, text);
    format_metric_bytes_gib(text, sizeof(text), snap->mem_used_b);
    set_text(s_ui.mem_used, text);
    format_metric_bytes_gib(text, sizeof(text), snap->mem_total_b);
    set_text(s_ui.mem_total, text);

    format_metric_percent(text, sizeof(text), snap->gpu_pct);
    set_text(s_ui.gpu_pct, text);
    format_metric_temp(text, sizeof(text), snap->gpu_temp_c);
    set_text(s_ui.gpu_temp, text);
    format_metric_power(text, sizeof(text), snap->gpu_power_w);
    set_text(s_ui.gpu_power, text);

    if (snap->history_valid) {
        charts_apply_history(snap);
    }
}

static void ui_set_loading_status(void)
{
    (void)0;
}

static void monitoring_ui_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    monitoring_ui_tick();
}

extern "C" esp_err_t monitoring_ui_init(void)
{
    if (s_ui_ready) {
        return ESP_OK;
    }
    if (!objects.ui_monitoring) {
        return ESP_ERR_INVALID_STATE;
    }

    lv_obj_t *screen = objects.ui_monitoring;
    lv_obj_clean(screen);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x0f1115), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(screen, action_ui_swipe, LV_EVENT_GESTURE, NULL);

    lv_obj_t *title = lv_label_create(screen);
    lv_obj_set_pos(title, 12, 8);
    lv_obj_set_style_text_font(title, &ui_font_ui_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_hex(0xd8deea), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(title, "Stats Linux Monitor");

    s_ui.host_meta = lv_label_create(screen);
    lv_obj_set_pos(s_ui.host_meta, 240, 12);
    lv_obj_set_size(s_ui.host_meta, 228, LV_SIZE_CONTENT);
    lv_obj_set_style_text_align(s_ui.host_meta, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(s_ui.host_meta, lv_color_hex(0x90a0bc), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(s_ui.host_meta, "host: -");

    lv_obj_t *cpu_panel = create_panel(screen, 6, 34, 154, 97);
    create_muted_label(cpu_panel, 0, 0, "CPU usage");
    s_ui.cpu_pct = create_value_label(cpu_panel, 0, 18);
    create_muted_label(cpu_panel, 0, 54, "Temp");
    s_ui.cpu_temp = create_value_label(cpu_panel, 50, 50);

    lv_obj_t *mem_panel = create_panel(screen, 163, 34, 154, 97);
    create_muted_label(mem_panel, 0, 0, "Memory");
    s_ui.mem_pct = create_value_label(mem_panel, 0, 18);
    create_muted_label(mem_panel, 0, 54, "Used");
    s_ui.mem_used = create_value_label(mem_panel, 45, 50);
    create_muted_label(mem_panel, 0, 72, "Total");
    s_ui.mem_total = create_value_label(mem_panel, 45, 68);

    lv_obj_t *gpu_panel = create_panel(screen, 320, 34, 154, 97);
    create_muted_label(gpu_panel, 0, 0, "GPU usage");
    s_ui.gpu_pct = create_value_label(gpu_panel, 0, 18);
    create_muted_label(gpu_panel, 0, 54, "Temp");
    s_ui.gpu_temp = create_value_label(gpu_panel, 48, 50);
    create_muted_label(gpu_panel, 0, 72, "Power");
    s_ui.gpu_power = create_value_label(gpu_panel, 48, 68);

    lv_obj_t *usage_panel = create_panel(screen, 6, 135, 232, 160);
    create_muted_label(usage_panel, 0, 0, "History CPU/GPU %");
    s_ui.usage_chart = lv_chart_create(usage_panel);
    lv_obj_set_pos(s_ui.usage_chart, 30, 18);
    lv_obj_set_size(s_ui.usage_chart, 186, 114);
    lv_obj_set_style_bg_color(s_ui.usage_chart, lv_color_hex(0x0f131d), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(s_ui.usage_chart, lv_color_hex(0x232a38), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(s_ui.usage_chart, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(s_ui.usage_chart, lv_color_hex(0x2a3140), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(s_ui.usage_chart, lv_color_hex(0x90a0bc), LV_PART_TICKS | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(s_ui.usage_chart, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_chart_set_type(s_ui.usage_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_div_line_count(s_ui.usage_chart, 5, 6);
    lv_chart_set_range(s_ui.usage_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_point_count(s_ui.usage_chart, MONITORING_HISTORY_POINTS_MAX);
    lv_obj_set_style_size(s_ui.usage_chart, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    s_ui.usage_cpu_series = lv_chart_add_series(s_ui.usage_chart, lv_color_hex(0x62a0ea), LV_CHART_AXIS_PRIMARY_Y);
    s_ui.usage_gpu_series = lv_chart_add_series(s_ui.usage_chart, lv_color_hex(0xf66151), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_ext_y_array(s_ui.usage_chart, s_ui.usage_cpu_series, s_usage_cpu_points);
    lv_chart_set_ext_y_array(s_ui.usage_chart, s_ui.usage_gpu_series, s_usage_gpu_points);
    for (size_t i = 0; i < MONITORING_Y_SCALE_LABEL_COUNT; ++i) {
        s_ui.usage_y_labels[i] = create_axis_value_label(usage_panel);
    }
    create_muted_label(usage_panel, 0, 136, "CPU bleu | GPU rouge");

    lv_obj_t *temp_panel = create_panel(screen, 242, 135, 232, 160);
    create_muted_label(temp_panel, 0, 0, "History CPU/GPU temperature");
    s_ui.temp_chart = lv_chart_create(temp_panel);
    lv_obj_set_pos(s_ui.temp_chart, 30, 18);
    lv_obj_set_size(s_ui.temp_chart, 186, 114);
    lv_obj_set_style_bg_color(s_ui.temp_chart, lv_color_hex(0x0f131d), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(s_ui.temp_chart, lv_color_hex(0x232a38), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(s_ui.temp_chart, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(s_ui.temp_chart, lv_color_hex(0x2a3140), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(s_ui.temp_chart, lv_color_hex(0x90a0bc), LV_PART_TICKS | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(s_ui.temp_chart, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_chart_set_type(s_ui.temp_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_div_line_count(s_ui.temp_chart, 5, 6);
    lv_chart_set_range(s_ui.temp_chart, LV_CHART_AXIS_PRIMARY_Y, 20, 90);
    lv_chart_set_point_count(s_ui.temp_chart, MONITORING_HISTORY_POINTS_MAX);
    lv_obj_set_style_size(s_ui.temp_chart, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    s_ui.temp_cpu_series = lv_chart_add_series(s_ui.temp_chart, lv_color_hex(0xffb84d), LV_CHART_AXIS_PRIMARY_Y);
    s_ui.temp_gpu_series = lv_chart_add_series(s_ui.temp_chart, lv_color_hex(0xf66151), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_ext_y_array(s_ui.temp_chart, s_ui.temp_cpu_series, s_temp_cpu_points);
    lv_chart_set_ext_y_array(s_ui.temp_chart, s_ui.temp_gpu_series, s_temp_gpu_points);
    for (size_t i = 0; i < MONITORING_Y_SCALE_LABEL_COUNT; ++i) {
        s_ui.temp_y_labels[i] = create_axis_value_label(temp_panel);
    }
    create_muted_label(temp_panel, 0, 136, "CPU orange | GPU rouge");

    s_ui.status = lv_label_create(screen);
    lv_obj_set_pos(s_ui.status, 8, 296);
    lv_obj_set_size(s_ui.status, 464, LV_SIZE_CONTENT);
    lv_obj_set_style_text_color(s_ui.status, lv_color_hex(0x90a0bc), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(s_ui.status, "");
    lv_obj_add_flag(s_ui.status, LV_OBJ_FLAG_HIDDEN);

    for (size_t i = 0; i < MONITORING_HISTORY_POINTS_MAX; ++i) {
        s_usage_cpu_points[i] = LV_CHART_POINT_NONE;
        s_usage_gpu_points[i] = LV_CHART_POINT_NONE;
        s_temp_cpu_points[i] = LV_CHART_POINT_NONE;
        s_temp_gpu_points[i] = LV_CHART_POINT_NONE;
    }
    lv_chart_refresh(s_ui.usage_chart);
    lv_chart_refresh(s_ui.temp_chart);
    update_chart_y_scale_labels(s_ui.usage_chart, s_ui.usage_y_labels, 0, 100);
    update_chart_y_scale_labels(s_ui.temp_chart, s_ui.temp_y_labels, 20, 90);

    s_ui_ready = true;
    s_force_refresh = true;
    s_last_active = false;
    ESP_LOGI(TAG, "UI monitoring initialisee");
    return ESP_OK;
}

extern "C" void monitoring_ui_start(void)
{
    if (!s_ui_ready || s_ui_timer) {
        return;
    }
    s_ui_timer = lv_timer_create(monitoring_ui_timer_cb, 200, NULL);
}

extern "C" void monitoring_ui_tick(void)
{
    if (!s_ui_ready || !objects.ui_monitoring) {
        return;
    }

    bool active = (lv_scr_act() == objects.ui_monitoring);
    if (active != s_last_active) {
        esp_err_t err = monitoring_service_set_active(active);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            ESP_LOGW(TAG, "set_active failed: %s", esp_err_to_name(err));
        }
        s_last_active = active;
        s_force_refresh = true;
        if (active) {
            ui_set_loading_status();
        }
    }

    if (!active) {
        return;
    }

    monitoring_snapshot_t snap = {};
    bool has_data = monitoring_service_get_snapshot(&snap);
    if (!has_data) {
        if (s_force_refresh) {
            ui_set_loading_status();
            s_force_refresh = false;
        }
        return;
    }

    if (s_force_refresh || snap.sequence != s_last_sequence) {
        apply_snapshot(&snap);
        s_last_sequence = snap.sequence;
        s_force_refresh = false;
    }
}
