#include "hourly_strip.h"

#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <cmath>
#include <lvgl.h>
#include "weather_fetcher.h"
#include "esp_log.h"
#include "ui/screens.h"
#include "ui/images.h"
#include "weather_icons.h"
#include "ui/fonts.h"
#include "vars.h"
#include "temp_unit.h"
extern "C" {
#include "time_sync.h"
}
#if HOURLY_STRIP_SIMULATION
#include "esp_system.h"
#include "esp_random.h"
#endif

#define HOURLY_STRIP_ICON_COUNT 7
#define HOURLY_CACHE_MAX 12

typedef struct {
    bool initialized = false;
    int last_hour = -1;
    int last_yday = -1;
    size_t hourly_count = 0;
    size_t hourly_cursor = 0;
    HourlyEntry hourly_cache[HOURLY_CACHE_MAX] = {};
    float temps[HOURLY_STRIP_ICON_COUNT] = {};
    bool detail_init_pending = false;
    bool detail_init_done = false;
    HourlyEntry history[2] = {};
    bool history_valid[2] = { false, false };
    bool last_unit_f = false;
#ifdef DEBUG_LOG
    const char *trace_source = NULL;
#endif
#if HOURLY_STRIP_SIMULATION
    int last_sim_stamp = -1;
    bool sim_pending = false;
    float sim_temp = NAN;
    int8_t sim_dir = 1;
    bool sim_active = false;
    time_t sim_base_ts = 0;
    size_t sim_icon_index = 0;
#endif
} HourlyStripState;

static HourlyStripState s_hourly = {};
static lv_chart_series_t *s_hourly_chart_series = NULL;
static lv_obj_t *s_hourly_chart = NULL;
static lv_obj_t *s_hourly_chart_line_top __attribute__((unused)) = NULL;
static lv_obj_t *s_hourly_chart_line_mid __attribute__((unused)) = NULL;
static lv_obj_t *s_hourly_chart_line_bottom __attribute__((unused)) = NULL;
static lv_obj_t *s_hourly_chart_label_top __attribute__((unused)) = NULL;
static lv_obj_t *s_hourly_chart_label_mid __attribute__((unused)) = NULL;
static lv_obj_t *s_hourly_chart_label_bottom __attribute__((unused)) = NULL;
static lv_point_t s_hourly_chart_line_points_top[2] __attribute__((unused));
static lv_point_t s_hourly_chart_line_points_mid[2] __attribute__((unused));
static lv_point_t s_hourly_chart_line_points_bottom[2] __attribute__((unused));
static lv_coord_t s_hourly_chart_series_1_array[HOURLY_STRIP_ICON_COUNT] = { 2, 5, 8, 9, 7, 6, 3 };

static void hourly_strip_apply_detail(time_t now_ts);
static void hourly_detail_apply_widgets(time_t now_ts);
static void hourly_strip_set_detail_vars(const HourlyEntry *entry);
static float __attribute__((unused)) hourly_strip_to_unit(float temp_c, bool is_fahrenheit);

#if HOURLY_STRIP_SIMULATION
static const struct {
    int conditionId;
    uint8_t iconVariant;
} s_hourly_sim_icons[] = {
    { 800, 0 }, { 801, 0 }, { 802, 0 }, { 803, 0 }, { 804, 0 },
    { 500, 0 }, { 501, 0 }, { 502, 0 }, { 600, 0 }, { 602, 0 },
    { 200, 0 }, { 210, 0 },
};
#endif

static void hourly_strip_history_reset(void)
{
    s_hourly.history_valid[0] = false;
    s_hourly.history_valid[1] = false;
}

static void hourly_strip_history_seed(const HourlyEntry *entry)
{
    if (!entry || !entry->valid) {
        return;
    }
    s_hourly.history[0] = *entry;
    s_hourly.history[1] = *entry;
    s_hourly.history_valid[0] = true;
    s_hourly.history_valid[1] = true;
}

static void hourly_strip_history_push(const HourlyEntry *entry)
{
    if (!entry || !entry->valid) {
        return;
    }
    s_hourly.history[1] = s_hourly.history[0];
    s_hourly.history_valid[1] = s_hourly.history_valid[0];
    s_hourly.history[0] = *entry;
    s_hourly.history_valid[0] = true;
}

static bool hourly_strip_chart_compute_range(lv_coord_t *out_min, lv_coord_t *out_max)
{
    if (!out_min || !out_max) {
        return false;
    }

    bool is_fahrenheit = temp_unit_is_fahrenheit();
    float min_temp = NAN;
    float max_temp = NAN;
    for (size_t i = 2; i < HOURLY_STRIP_ICON_COUNT; ++i) {
        float temp = s_hourly.temps[i];
        if (std::isnan(temp)) {
            continue;
        }
        float display = hourly_strip_to_unit(temp, is_fahrenheit);
        if (std::isnan(min_temp) || display < min_temp) {
            min_temp = display;
        }
        if (std::isnan(max_temp) || display > max_temp) {
            max_temp = display;
        }
    }

    if (std::isnan(min_temp) || std::isnan(max_temp)) {
        return false;
    }

    float step = is_fahrenheit ? 10.0f : 5.0f;
    float pad = step;
    int32_t min_floor = (int32_t)std::floor((min_temp - pad) / step) * (int32_t)step;
    int32_t max_ceil = (int32_t)std::ceil((max_temp + pad) / step) * (int32_t)step;

    int32_t range_min = min_floor;
    int32_t span = is_fahrenheit ? 40 : 20;
    int32_t range_max = range_min + span;
    if (max_ceil > range_max) {
        range_max = max_ceil;
        range_min = range_max - span;
    }

    *out_min = (lv_coord_t)range_min;
    *out_max = (lv_coord_t)range_max;
    return true;
}

static void hourly_strip_chart_sync(void)
{
    bool is_fahrenheit = temp_unit_is_fahrenheit();
    for (size_t i = 0; i < HOURLY_STRIP_ICON_COUNT; ++i) {
        float temp = s_hourly.temps[i];
        if (std::isnan(temp)) {
            s_hourly_chart_series_1_array[i] = 0;
        } else {
            float display = hourly_strip_to_unit(temp, is_fahrenheit);
            s_hourly_chart_series_1_array[i] = (lv_coord_t)std::lround(display);
        }
    }

    if (s_hourly_chart && s_hourly_chart_series) {
        lv_coord_t range_min = 0;
        lv_coord_t range_max = 0;
        if (hourly_strip_chart_compute_range(&range_min, &range_max)) {
            lv_chart_set_range(s_hourly_chart, LV_CHART_AXIS_PRIMARY_Y, range_min, range_max);
#ifdef DEBUG_LOG
            char label_buf[128];
            int offset = snprintf(label_buf, sizeof(label_buf), "[");
            if (offset < 0) {
                label_buf[0] = '\0';
            } else {
                int step = temp_unit_is_fahrenheit() ? 10 : 5;
                for (int val = (int)range_max; val >= (int)range_min; val -= step) {
                    int written = snprintf(label_buf + offset, sizeof(label_buf) - (size_t)offset,
                                           "%s%d", (offset > 1) ? " " : "", val);
                    if (written < 0 || (size_t)written >= sizeof(label_buf) - (size_t)offset) {
                        break;
                    }
                    offset += written;
                }
                if ((size_t)offset < sizeof(label_buf) - 1) {
                    label_buf[offset++] = ']';
                    label_buf[offset] = '\0';
                } else {
                    label_buf[sizeof(label_buf) - 1] = '\0';
                }
            }
            ESP_LOGI("HOURLY", "new val : %d : range Y %s", (int)s_hourly_chart_series_1_array[2], label_buf);
#endif
        }
        lv_chart_refresh(s_hourly_chart);
    }
}

static void hourly_strip_refresh_unit_if_needed(void)
{
    bool is_fahrenheit = temp_unit_is_fahrenheit();
    if (s_hourly.last_unit_f == is_fahrenheit) {
        return;
    }
    s_hourly.last_unit_f = is_fahrenheit;
    hourly_strip_chart_sync();
}

static void hourly_strip_apply_detail(time_t now_ts)
{
#ifdef DEBUG_LOG
    s_hourly.trace_source = "apply_detail";
#endif
    const HourlyEntry *detail_entry = NULL;
    size_t detail_index = s_hourly.hourly_cursor + 2;
    if (s_hourly.hourly_count > detail_index &&
        s_hourly.hourly_cache[detail_index].valid) {
        detail_entry = &s_hourly.hourly_cache[detail_index];
    }
    hourly_strip_set_detail_vars(detail_entry);
    hourly_detail_apply_widgets(now_ts);
}

static void hourly_strip_chart_draw_cb(lv_event_t *event)
{
    lv_obj_draw_part_dsc_t *dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(event);
    if (!dsc) {
        return;
    }
    if (dsc->part != LV_PART_TICKS) {
        return;
    }
    if (dsc->type != LV_CHART_DRAW_PART_TICK_LABEL) {
        return;
    }
    if (dsc->id != LV_CHART_AXIS_PRIMARY_Y) {
        return;
    }
    if (dsc->value != 0 || !dsc->p1) {
        return;
    }

    lv_obj_t *chart = lv_event_get_target(event);
    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.color = lv_palette_main(LV_PALETTE_BLUE);
    line_dsc.width = 2;
    line_dsc.opa = LV_OPA_50;

    lv_point_t p1 = { chart->coords.x1, dsc->p1->y };
    lv_point_t p2 = { chart->coords.x2, dsc->p1->y };
    lv_draw_line(dsc->draw_ctx, &line_dsc, &p1, &p2);
}

static lv_obj_t *hourly_strip_create_chart(lv_obj_t *parent)
{
    if (!parent) {
        return NULL;
    }

    lv_obj_update_layout(parent);
    lv_coord_t parent_w = lv_obj_get_width(parent);
    if (parent_w > 400) {
        parent_w = 400;
    }

    lv_obj_t *chart = lv_chart_create(parent);
    lv_obj_set_width(chart, parent_w);
    lv_obj_set_height(chart, 100);
    lv_obj_set_pos(chart, 10, 0);
    lv_obj_set_align(chart, LV_ALIGN_CENTER);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(chart, HOURLY_STRIP_ICON_COUNT);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 20);
    lv_chart_set_range(chart, LV_CHART_AXIS_SECONDARY_Y, 0, 0);
    lv_chart_set_div_line_count(chart, 5, HOURLY_STRIP_ICON_COUNT);
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 10, 2, HOURLY_STRIP_ICON_COUNT, 3, false, 50);
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 10, 5, 5, 2, true, 50);
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_SECONDARY_Y, 0, 0, 0, 0, false, 25);
    lv_obj_set_style_bg_color(chart, lv_color_hex(0x141b1e), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(chart, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(chart, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(chart, lv_color_hex(0x2a3336), LV_PART_MAIN);
    lv_obj_set_style_line_color(chart, lv_color_hex(0x263034), LV_PART_MAIN);
    lv_obj_set_style_text_color(chart, lv_color_hex(0xd0d0d0), LV_PART_TICKS);
    lv_obj_set_style_text_font(chart, &ui_font_ui_16, LV_PART_TICKS);

    s_hourly_chart_series = lv_chart_add_series(chart, lv_color_hex(0xb7b7b7), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_series_color(chart, s_hourly_chart_series, lv_color_hex(0xb7b7b7));
    lv_obj_set_style_bg_color(chart, lv_color_hex(0xc2c2c2), LV_PART_INDICATOR);
    lv_obj_set_style_border_color(chart, lv_color_hex(0x9fa4a6), LV_PART_INDICATOR);
    lv_obj_set_style_border_width(chart, 1, LV_PART_INDICATOR);
    lv_obj_set_style_size(chart, 6, LV_PART_INDICATOR);
    lv_chart_set_ext_y_array(chart, s_hourly_chart_series, s_hourly_chart_series_1_array);
    lv_obj_add_event_cb(chart, hourly_strip_chart_draw_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

    return chart;
}

static void hourly_strip_detail_chart_refresh(void)
{
    if (s_hourly_chart && !lv_obj_is_valid(s_hourly_chart)) {
        s_hourly_chart = NULL;
        s_hourly_chart_series = NULL;
    }

    lv_obj_t *parent = objects.ui_detail_chart;
    if (!parent) {
        return;
    }

    if (s_hourly_chart && lv_obj_get_parent(s_hourly_chart) == parent) {
        return;
    }

    if (s_hourly_chart) {
        lv_obj_del(s_hourly_chart);
        s_hourly_chart = NULL;
        s_hourly_chart_series = NULL;
    }

    s_hourly_chart = hourly_strip_create_chart(parent);
    hourly_strip_chart_sync();
#ifdef DEBUG_LOG
    if (s_hourly_chart) {
        ESP_LOGI("HOURLY", "Chart horaire cree dans ui_detail_chart");
    } else {
        ESP_LOGW("HOURLY", "Chart horaire non cree (parent nul)");
    }
#endif
}

static bool hourly_strip_ready(void)
{
    return objects.ui_detail_hourly != NULL;
}

static float __attribute__((unused)) hourly_strip_to_unit(float temp_c, bool is_fahrenheit)
{
    if (std::isnan(temp_c)) {
        return temp_c;
    }
    if (!is_fahrenheit) {
        return temp_c;
    }
    return (temp_c * 9.0f / 5.0f) + 32.0f;
}

static float __attribute__((unused)) hourly_strip_round_step(float value, float step)
{
    if (std::isnan(value) || step <= 0.0f) {
        return value;
    }
    return step * std::round(value / step);
}

static void __attribute__((unused)) hourly_strip_chart_set_line(lv_obj_t *line,
                                                                lv_point_t *points,
                                                                lv_coord_t x,
                                                                lv_coord_t y,
                                                                lv_coord_t width)
{
    if (!line || !points) {
        return;
    }
    points[0].x = 0;
    points[0].y = 0;
    points[1].x = width;
    points[1].y = 0;
    lv_line_set_points(line, points, 2);
    lv_obj_set_pos(line, x, y);
}

static void __attribute__((unused)) hourly_strip_chart_set_label(lv_obj_t *label, const char *text, lv_coord_t x, lv_coord_t y)
{
    if (!label) {
        return;
    }
    lv_label_set_text(label, text ? text : "");
    lv_obj_update_layout(label);
    lv_coord_t h = lv_obj_get_height(label);
    lv_obj_set_pos(label, x, (lv_coord_t)(y - h / 2));
}

static float hourly_strip_temp_from_entry(const HourlyEntry *entry, float fallback)
{
    if (entry && entry->valid && !std::isnan(entry->temperature)) {
        return entry->temperature;
    }
#ifdef DEBUG_LOG
    if (entry && !entry->valid) {
        ESP_LOGW("HOURLY", "erreur data hourly: entry invalide (temp)");
    }
#endif
    return fallback;
}

static lv_obj_t *hourly_detail_widget_container(size_t index)
{
    switch (index) {
    case 0:
        return objects.obj11;
    case 1:
        return objects.obj12;
    case 2:
        return objects.obj13;
    case 3:
        return objects.obj14;
    case 4:
        return objects.obj15;
    case 5:
        return objects.obj16;
    case 6:
        return objects.obj17;
    default:
        return NULL;
    }
}

static lv_obj_t *hourly_detail_widget_label(size_t index)
{
    switch (index) {
    case 0:
        return objects.obj11__obj0;
    case 1:
        return objects.obj12__obj0;
    case 2:
        return objects.obj13__obj0;
    case 3:
        return objects.obj14__obj0;
    case 4:
        return objects.obj15__obj0;
    case 5:
        return objects.obj16__obj0;
    case 6:
        return objects.obj17__obj0;
    default:
        return NULL;
    }
}

static void hourly_detail_set_widget(size_t index,
                                     const HourlyEntry *entry,
                                     const HourlyEntry *icon_entry,
                                     time_t fallback_ts)
{
    lv_obj_t *container = hourly_detail_widget_container(index);
    lv_obj_t *label = hourly_detail_widget_label(index);
    if (!container || !label) {
        return;
    }

    lv_obj_t *icon = lv_obj_get_child(container, 0);
    time_t ts = fallback_ts;
    time_t entry_ts = 0;
    if (entry && entry->valid) {
        entry_ts = entry->timestamp;
        ts = entry->timestamp;
#ifdef DEBUG_LOG
    } else if (entry && !entry->valid) {
        ESP_LOGW("HOURLY", "erreur data hourly: entry invalide (ts) slot=%u", (unsigned)index);
#endif
    }

    const time_sync_config_t *time_cfg = time_sync_get_config();
    int32_t tz_offset = time_cfg ? time_cfg->tz_offset_seconds : 0;
    time_t display_ts = ts ? (ts + (time_t)tz_offset) : ts;

    struct tm info;
    if (display_ts > 0 && localtime_r(&display_ts, &info)) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%02dH", info.tm_hour);
        buf[sizeof(buf) - 1] = '\0';
        lv_label_set_text(label, buf);
    } else {
        lv_label_set_text(label, "");
    }

    if (!icon) {
        return;
    }

    const HourlyEntry *icon_src = icon_entry ? icon_entry : entry;
    if (icon_src && icon_src->valid && icon_src->conditionId != 0) {
        weather_icons_set_object(icon, "icon_50.bin", (uint16_t)icon_src->conditionId, icon_src->iconVariant);
    } else {
        lv_img_set_src(icon, &img_clear_day_50);
    }

#ifdef DEBUG_LOG
    const char *label_text = lv_label_get_text(label);
    int dt_hh = -1;
    if (display_ts > 0 && localtime_r(&display_ts, &info)) {
        dt_hh = info.tm_hour;
    }
    const char *icon_id = "";
    int condition_id = 0;
    if (icon_src && icon_src->valid) {
        icon_id = icon_src->iconId.c_str();
        condition_id = icon_src->conditionId;
    }
    float entry_temp = NAN;
    if (entry && entry->valid) {
        entry_temp = entry->temperature;
    }
    const char *entry_valid = (entry && entry->valid) ? "1" : "0";
    const char *icon_valid = (icon_src && icon_src->valid) ? "1" : "0";
    ESP_LOGI("HOURLY",
             "ui_meteo_detail slot=%u id=%d icon_id=%s dt_hh=%d label=%s temp=%.2f ts=%ld entry_ts=%ld fallback_ts=%ld entry_valid=%s icon_valid=%s",
             (unsigned)index,
             condition_id,
             icon_id ? icon_id : "",
             dt_hh,
             label_text ? label_text : "",
             (double)entry_temp,
             (long)ts,
             (long)entry_ts,
             (long)fallback_ts,
             entry_valid,
             icon_valid);
#endif
}

static void hourly_detail_apply_widgets(time_t now_ts)
{
    if (!objects.ui_detail_hourly) {
        return;
    }

#ifdef DEBUG_LOG
    const char *trace_src = s_hourly.trace_source ? s_hourly.trace_source : "unknown";
#endif

    if (!now_ts) {
        now_ts = time(NULL);
    }

    const HourlyEntry *current_entry = NULL;
    if (s_hourly.hourly_count > 0 &&
        s_hourly.hourly_cursor < s_hourly.hourly_count &&
        s_hourly.hourly_cache[s_hourly.hourly_cursor].valid) {
        current_entry = &s_hourly.hourly_cache[s_hourly.hourly_cursor];
    }

    time_t base_ts = now_ts;
    if (current_entry && current_entry->timestamp > 0) {
        base_ts = current_entry->timestamp;
    }

#ifdef DEBUG_LOG
    ESP_LOGI("HOURLY",
             "ui_meteo_detail_trace src=%s now_ts=%ld base_ts=%ld cursor=%u count=%u",
             trace_src,
             (long)now_ts,
             (long)base_ts,
             (unsigned)s_hourly.hourly_cursor,
             (unsigned)s_hourly.hourly_count);
#endif

    for (size_t slot = 0; slot < HOURLY_STRIP_ICON_COUNT; ++slot) {
        const HourlyEntry *entry = NULL;
        int idx = (int)s_hourly.hourly_cursor + ((int)slot - 2);
        if (idx >= 0 && (size_t)idx < s_hourly.hourly_count) {
            entry = &s_hourly.hourly_cache[idx];
        }
        time_t fallback_ts = 0;
        if (base_ts) {
            int offset_hours = (int)slot - 2;
            fallback_ts = base_ts + (time_t)(offset_hours * 3600);
        }
#ifdef DEBUG_LOG
        ESP_LOGI("HOURLY",
                 "ui_meteo_detail_trace src=%s slot=%u idx=%d fallback_ts=%ld entry=%s",
                 trace_src,
                 (unsigned)slot,
                 idx,
                 (long)fallback_ts,
                 entry ? "1" : "0");
#endif
        const HourlyEntry *icon_entry = entry;
        if (slot == 0 && s_hourly.history_valid[1]) {
            icon_entry = &s_hourly.history[1];
        } else if (slot == 1 && s_hourly.history_valid[0]) {
            icon_entry = &s_hourly.history[0];
        } else if (s_hourly.detail_init_pending && slot < 2 && current_entry) {
            icon_entry = current_entry;
        }
        hourly_detail_set_widget(slot, entry, icon_entry, fallback_ts);
#ifdef DEBUG_LOG
        if (slot == 0) {
            char log_buf[128];
            int offset = snprintf(log_buf, sizeof(log_buf), "Icons hourly:");
            if (offset < 0) {
                log_buf[0] = '\0';
            }
            for (size_t i = 0; i < HOURLY_STRIP_ICON_COUNT; ++i) {
                const HourlyEntry *e = NULL;
                int idx2 = (int)s_hourly.hourly_cursor + ((int)i - 2);
                if (idx2 >= 0 && (size_t)idx2 < s_hourly.hourly_count) {
                    e = &s_hourly.hourly_cache[idx2];
                }
                const HourlyEntry *icon_src = e;
                if (i == 0 && s_hourly.history_valid[1]) {
                    icon_src = &s_hourly.history[1];
                } else if (i == 1 && s_hourly.history_valid[0]) {
                    icon_src = &s_hourly.history[0];
                } else if (s_hourly.detail_init_pending && i < 2 && current_entry) {
                    icon_src = current_entry;
                }
                int code = (icon_src && icon_src->valid) ? icon_src->conditionId : 0;
                if (offset > 0 && (size_t)offset < sizeof(log_buf)) {
                    int written = snprintf(log_buf + offset,
                                           sizeof(log_buf) - (size_t)offset,
                                           " %d",
                                           code);
                    if (written < 0 || (size_t)written >= sizeof(log_buf) - (size_t)offset) {
                        break;
                    }
                    offset += written;
                }
            }
            ESP_LOGI("HOURLY", "%s", log_buf);
        }
#endif
    }

    if (s_hourly.detail_init_pending && current_entry) {
        hourly_strip_history_seed(current_entry);
        s_hourly.detail_init_pending = false;
        s_hourly.detail_init_done = true;
    }
}

static int hourly_strip_percent_from_pop(float pop)
{
    if (std::isnan(pop)) {
        return -1;
    }
    int value = (int)std::lround(pop * 100.0f);
    if (value < 0) {
        value = 0;
    } else if (value > 100) {
        value = 100;
    }
    return value;
}

static void hourly_strip_set_detail_vars(const HourlyEntry *entry)
{
    char buf[16];
    if (entry && entry->valid) {
        snprintf(buf, sizeof(buf), "%u%%", (unsigned)entry->humidity);
        buf[sizeof(buf) - 1] = '\0';
        set_var_ui_humidity(buf);

        snprintf(buf, sizeof(buf), "%u%%", (unsigned)entry->clouds);
        buf[sizeof(buf) - 1] = '\0';
        set_var_ui_clouds(buf);

        int pop_percent = hourly_strip_percent_from_pop(entry->pop);
        if (pop_percent >= 0) {
            snprintf(buf, sizeof(buf), "%d", pop_percent);
        } else {
            buf[0] = '\0';
        }
        buf[sizeof(buf) - 1] = '\0';
        set_var_ui_pop(buf);
        return;
    }

#ifdef DEBUG_LOG
    if (entry && !entry->valid) {
        ESP_LOGW("HOURLY", "erreur data hourly: entry invalide (detail vars)");
    }
#endif
    set_var_ui_humidity("");
    set_var_ui_clouds("");
    set_var_ui_pop("");
}

static size_t hourly_strip_find_cursor(time_t now_ts)
{
    (void)now_ts;
    return 0;
}

static float hourly_strip_next_temp(void)
{
#if HOURLY_STRIP_SIMULATION
    if (s_hourly.sim_pending) {
        return s_hourly.sim_temp;
    }
#endif
    float fallback = s_hourly.temps[2];
    size_t next_index = s_hourly.hourly_cursor + 4;
    if (next_index < s_hourly.hourly_count) {
        return hourly_strip_temp_from_entry(&s_hourly.hourly_cache[next_index], fallback);
    }
    return fallback;
}

static void hourly_strip_shift_state(const float *new_temp)
{
    for (size_t i = 0; i + 1 < HOURLY_STRIP_ICON_COUNT; ++i) {
        s_hourly.temps[i] = s_hourly.temps[i + 1];
    }
    if (new_temp) {
        s_hourly.temps[HOURLY_STRIP_ICON_COUNT - 1] = *new_temp;
    }
#if HOURLY_STRIP_SIMULATION
    s_hourly.sim_pending = false;
#endif
    if (s_hourly.hourly_count > 0 &&
        s_hourly.hourly_cursor + 1 < s_hourly.hourly_count) {
#if HOURLY_STRIP_SIMULATION
        if (!s_hourly.sim_active) {
            s_hourly.hourly_cursor++;
        }
#else
        s_hourly.hourly_cursor++;
#endif
    }
}

static void hourly_strip_shift_no_anim(time_t now_ts, bool push_history)
{
    if (push_history) {
        if (s_hourly.hourly_count > 0 &&
            s_hourly.hourly_cursor < s_hourly.hourly_count &&
            s_hourly.hourly_cache[s_hourly.hourly_cursor].valid) {
            hourly_strip_history_push(&s_hourly.hourly_cache[s_hourly.hourly_cursor]);
        }
    }
    float new_temp = hourly_strip_next_temp();
    hourly_strip_shift_state(&new_temp);
    hourly_strip_chart_sync();
    hourly_strip_apply_detail(now_ts);
}

#if HOURLY_STRIP_SIMULATION
static void hourly_strip_sim_fill_entry(HourlyEntry *entry, time_t ts, size_t icon_index)
{
    if (!entry) {
        return;
    }
    const size_t icon_count = sizeof(s_hourly_sim_icons) / sizeof(s_hourly_sim_icons[0]);
    size_t idx = (icon_index < icon_count) ? icon_index : (icon_index % icon_count);
    entry->timestamp = ts;
    entry->temperature = s_hourly.sim_temp;
    entry->feelsLike = s_hourly.sim_temp;
    entry->pop = 0.15f;
    entry->humidity = 55;
    entry->clouds = 35;
    entry->rain1h = NAN;
    entry->snow1h = NAN;
    entry->iconId.clear();
    entry->iconVariant = s_hourly_sim_icons[idx].iconVariant;
    entry->conditionId = s_hourly_sim_icons[idx].conditionId;
    entry->valid = true;
}

static void hourly_strip_sim_seed(time_t now_ts)
{
    if (s_hourly.sim_active) {
        return;
    }
    if (!now_ts) {
        now_ts = time(NULL);
    }
    s_hourly.sim_base_ts = now_ts;
    s_hourly.sim_icon_index = 0;
    s_hourly.hourly_count = HOURLY_CACHE_MAX;
    s_hourly.hourly_cursor = 0;
    for (size_t i = 0; i < HOURLY_CACHE_MAX; ++i) {
        time_t ts = now_ts + (time_t)(i * 3600);
        hourly_strip_sim_fill_entry(&s_hourly.hourly_cache[i], ts, s_hourly.sim_icon_index++);
    }
    s_hourly.sim_active = true;
    hourly_strip_apply_detail(now_ts);
}

static void hourly_strip_sim_shift_cache(time_t now_ts)
{
    if (!s_hourly.sim_active) {
        hourly_strip_sim_seed(now_ts);
        return;
    }
    if (s_hourly.hourly_count == 0) {
        s_hourly.hourly_count = HOURLY_CACHE_MAX;
    }
    for (size_t i = 0; i + 1 < s_hourly.hourly_count; ++i) {
        s_hourly.hourly_cache[i] = s_hourly.hourly_cache[i + 1];
    }
    s_hourly.sim_base_ts += 3600;
    time_t ts = s_hourly.sim_base_ts + (time_t)((s_hourly.hourly_count - 1) * 3600);
    hourly_strip_sim_fill_entry(&s_hourly.hourly_cache[s_hourly.hourly_count - 1],
                                ts,
                                s_hourly.sim_icon_index++);
    s_hourly.hourly_cursor = 0;
}

static float hourly_strip_sim_next_temp(void)
{
    if (std::isnan(s_hourly.sim_temp)) {
        s_hourly.sim_temp = -25.0f;
        s_hourly.sim_dir = 1;
    }

    float next = s_hourly.sim_temp + (float)(2 * s_hourly.sim_dir);
    if (next >= 25.0f) {
        next = 25.0f;
        s_hourly.sim_dir = -1;
    } else if (next <= -25.0f) {
        next = -25.0f;
        s_hourly.sim_dir = 1;
    }

    return next;
}

static void hourly_strip_sim_tick(const struct tm *timeinfo, bool details_active)
{
    if (!timeinfo || !hourly_strip_ready() || !s_hourly.initialized) {
        return;
    }

    if (std::isnan(s_hourly.sim_temp)) {
        s_hourly.sim_temp = -25.0f;
        s_hourly.sim_dir = 1;
        float temp_seed = s_hourly.sim_temp;
        for (size_t i = 0; i < HOURLY_STRIP_ICON_COUNT; ++i) {
            s_hourly.temps[i] = temp_seed;
        }
    }

    struct tm timeinfo_copy = *timeinfo;
    time_t now_ts = mktime(&timeinfo_copy);
    if (now_ts == (time_t)-1) {
        now_ts = 0;
    }

    int stamp = timeinfo->tm_min * 60 + timeinfo->tm_sec;
    if (stamp == s_hourly.last_sim_stamp) {
        return;
    }
    s_hourly.last_sim_stamp = stamp;
    if ((timeinfo->tm_sec % 30) != 0) {
        return;
    }

    if (s_hourly.sim_active &&
        s_hourly.hourly_count > 0 &&
        s_hourly.hourly_cache[0].valid) {
        hourly_strip_history_push(&s_hourly.hourly_cache[0]);
    }
    s_hourly.sim_temp = hourly_strip_sim_next_temp();
    s_hourly.sim_pending = true;
    (void)details_active;
#ifdef DEBUG_LOG
    s_hourly.trace_source = "sim_tick";
#endif
    hourly_strip_sim_shift_cache(now_ts);
    hourly_strip_shift_no_anim(now_ts, false);
}
#endif

void hourly_strip_update(const CurrentWeatherData *current,
                         const HourlyEntry *hourly,
                         size_t hourly_count,
                         time_t now_ts)
{
    if (!hourly_strip_ready()) {
        return;
    }

    hourly_strip_refresh_unit_if_needed();

    if (!hourly || hourly_count == 0) {
#ifdef DEBUG_LOG
        s_hourly.trace_source = "update_empty";
        ESP_LOGW("HOURLY", "erreur data hourly: cache vide, affichage conserve");
#endif
        return;
    }

    if (!s_hourly.initialized) {
        for (size_t i = 0; i < HOURLY_STRIP_ICON_COUNT; ++i) {
            s_hourly.temps[i] = NAN;
        }
        s_hourly.initialized = true;
        hourly_strip_history_reset();
    }

    size_t copy = hourly_count < HOURLY_CACHE_MAX ? hourly_count : HOURLY_CACHE_MAX;
    for (size_t i = 0; i < copy; ++i) {
        s_hourly.hourly_cache[i] = hourly[i];
    }
    s_hourly.hourly_count = copy;
    s_hourly.hourly_cursor = hourly_strip_find_cursor(now_ts);

#ifdef DEBUG_LOG
    for (size_t i = 0; i < s_hourly.hourly_count; ++i) {
        ESP_LOGI("HOURLY",
                 "cache_hourly i=%u dt=%ld temp=%.2f pop=%.2f valid=%d",
                 (unsigned)i,
                 (long)s_hourly.hourly_cache[i].timestamp,
                 (double)s_hourly.hourly_cache[i].temperature,
                 (double)s_hourly.hourly_cache[i].pop,
                 s_hourly.hourly_cache[i].valid ? 1 : 0);
    }
#endif

#ifdef DEBUG_LOG
    s_hourly.trace_source = "update";
#endif
    bool first_update = !s_hourly.detail_init_done;
    hourly_strip_apply_detail(now_ts);

    float now_temp = NAN;
    if (current && !std::isnan(current->temperature)) {
        now_temp = current->temperature;
    }
    float fallback_temp = now_temp;
    if (std::isnan(fallback_temp)) {
        fallback_temp = s_hourly.temps[2];
    }
    if (s_hourly.detail_init_done) {
        s_hourly.temps[0] = s_hourly.temps[1];
        s_hourly.temps[1] = s_hourly.temps[2];
    }
    s_hourly.temps[2] = fallback_temp;

    if (first_update) {
        float init_ref = fallback_temp;
        if (s_hourly.hourly_count > s_hourly.hourly_cursor) {
            init_ref = hourly_strip_temp_from_entry(&s_hourly.hourly_cache[s_hourly.hourly_cursor], fallback_temp);
        }
        s_hourly.temps[0] = init_ref;
        s_hourly.temps[1] = init_ref;
    }

    for (size_t i = 0; i < 4; ++i) {
        size_t idx = s_hourly.hourly_cursor + i;
        if (idx < s_hourly.hourly_count) {
            s_hourly.temps[3 + i] = hourly_strip_temp_from_entry(&s_hourly.hourly_cache[idx], fallback_temp);
        } else {
            s_hourly.temps[3 + i] = 0.0f;
        }
    }

    hourly_strip_chart_sync();

    if (!now_ts) {
        now_ts = time(NULL);
    }
    if (now_ts) {
        struct tm now_info;
        if (localtime_r(&now_ts, &now_info)) {
            s_hourly.last_hour = now_info.tm_hour;
            s_hourly.last_yday = now_info.tm_yday;
        }
    }
}

void hourly_strip_tick(const struct tm *timeinfo, bool details_active)
{
    if (!timeinfo || !hourly_strip_ready() || !s_hourly.initialized) {
        return;
    }

    if (details_active) {
#ifdef DEBUG_LOG
        s_hourly.trace_source = "tick_detail";
#endif
        hourly_strip_detail_chart_refresh();
    }
    hourly_strip_refresh_unit_if_needed();
#if HOURLY_STRIP_SIMULATION
    hourly_strip_sim_tick(timeinfo, details_active);
    return;
#endif
    if (s_hourly.hourly_count == 0) {
        return;
    }
    if (timeinfo->tm_min != 0) {
        return;
    }
    if (s_hourly.last_hour == timeinfo->tm_hour &&
        s_hourly.last_yday == timeinfo->tm_yday) {
        return;
    }

    s_hourly.last_hour = timeinfo->tm_hour;
    s_hourly.last_yday = timeinfo->tm_yday;

    struct tm timeinfo_copy = *timeinfo;
    time_t now_ts = mktime(&timeinfo_copy);
    if (now_ts == (time_t)-1) {
        now_ts = 0;
    }
#ifdef DEBUG_LOG
    s_hourly.trace_source = "tick_shift";
#endif
    hourly_strip_shift_no_anim(now_ts, true);
}

extern "C" void hourly_strip_detail_chart_ensure(void)
{
    hourly_strip_detail_chart_refresh();
}

extern "C" void hourly_strip_detail_ui_init(void)
{
    if (s_hourly.detail_init_done || s_hourly.detail_init_pending) {
        return;
    }
    s_hourly.detail_init_pending = true;
#ifdef DEBUG_LOG
    s_hourly.trace_source = "detail_ui_init";
#endif
    if (s_hourly.hourly_count > 0) {
        hourly_detail_apply_widgets(time(NULL));
#ifdef DEBUG_LOG
    } else {
        ESP_LOGI("HOURLY", "detail_ui_init: hourly vide, update differe");
#endif
    }
}
