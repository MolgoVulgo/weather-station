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
#include "ui/fonts.h"
#include "vars.h"
#include "temp_unit.h"
#ifdef HOURLY_STRIP_SIMULATION
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
#ifdef HOURLY_STRIP_SIMULATION
    int last_sim_stamp = -1;
    bool sim_pending = false;
    float sim_temp = NAN;
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
    return fallback;
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
        set_var_ui_humidity((int32_t)(intptr_t)buf);

        snprintf(buf, sizeof(buf), "%u%%", (unsigned)entry->clouds);
        buf[sizeof(buf) - 1] = '\0';
        set_var_ui_clouds((int32_t)(intptr_t)buf);

        int pop_percent = hourly_strip_percent_from_pop(entry->pop);
        if (pop_percent >= 0) {
            snprintf(buf, sizeof(buf), "%d%%", pop_percent);
        } else {
            buf[0] = '\0';
        }
        buf[sizeof(buf) - 1] = '\0';
        set_var_ui_pop((int32_t)(intptr_t)buf);
        return;
    }

    set_var_ui_humidity((int32_t)(intptr_t)"");
    set_var_ui_clouds((int32_t)(intptr_t)"");
    set_var_ui_pop((int32_t)(intptr_t)"");
}

static size_t hourly_strip_find_cursor(time_t now_ts)
{
    if (s_hourly.hourly_count == 0) {
        return 0;
    }
    for (size_t i = 0; i < s_hourly.hourly_count; ++i) {
        if (s_hourly.hourly_cache[i].valid &&
            s_hourly.hourly_cache[i].timestamp > now_ts) {
            return i;
        }
    }
    return 0;
}

static float hourly_strip_next_temp(void)
{
#ifdef HOURLY_STRIP_SIMULATION
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
#ifdef HOURLY_STRIP_SIMULATION
    s_hourly.sim_pending = false;
#endif
    if (s_hourly.hourly_count > 0 &&
        s_hourly.hourly_cursor + 1 < s_hourly.hourly_count) {
        s_hourly.hourly_cursor++;
    }
}

static void hourly_strip_shift_no_anim(void)
{
    float new_temp = hourly_strip_next_temp();
    hourly_strip_shift_state(&new_temp);
}

#ifdef HOURLY_STRIP_SIMULATION
static float hourly_strip_random_temp(void)
{
    uint32_t r = esp_random();
    int32_t temp = (int32_t)(r % 45) - 5;
    return (float)temp;
}

static void hourly_strip_sim_tick(const struct tm *timeinfo, bool details_active)
{
    if (!timeinfo || !hourly_strip_ready() || !s_hourly.initialized) {
        return;
    }

    if (std::isnan(s_hourly.temps[2])) {
        float temp_seed = hourly_strip_random_temp();
        for (size_t i = 0; i < HOURLY_STRIP_ICON_COUNT; ++i) {
            s_hourly.temps[i] = temp_seed;
        }
    }

    int stamp = timeinfo->tm_min * 60 + timeinfo->tm_sec;
    if (stamp == s_hourly.last_sim_stamp) {
        return;
    }
    s_hourly.last_sim_stamp = stamp;
    if ((timeinfo->tm_sec % 30) != 0) {
        return;
    }

    s_hourly.sim_temp = hourly_strip_random_temp();
    s_hourly.sim_pending = true;
    (void)details_active;
    hourly_strip_shift_no_anim();
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

    if (!s_hourly.initialized) {
        for (size_t i = 0; i < HOURLY_STRIP_ICON_COUNT; ++i) {
            s_hourly.temps[i] = NAN;
        }
        s_hourly.initialized = true;
    }

    if (hourly && hourly_count > 0) {
        size_t copy = hourly_count < HOURLY_CACHE_MAX ? hourly_count : HOURLY_CACHE_MAX;
        for (size_t i = 0; i < copy; ++i) {
            s_hourly.hourly_cache[i] = hourly[i];
        }
        s_hourly.hourly_count = copy;
        s_hourly.hourly_cursor = hourly_strip_find_cursor(now_ts);
    } else {
        s_hourly.hourly_count = 0;
        s_hourly.hourly_cursor = 0;
    }

    const HourlyEntry *detail_entry = NULL;
    if (s_hourly.hourly_count > 0 &&
        s_hourly.hourly_cursor < s_hourly.hourly_count &&
        s_hourly.hourly_cache[s_hourly.hourly_cursor].valid) {
        detail_entry = &s_hourly.hourly_cache[s_hourly.hourly_cursor];
    }
    hourly_strip_set_detail_vars(detail_entry);

    float now_temp = NAN;
    if (current && !std::isnan(current->temperature)) {
        now_temp = current->temperature;
    }
    float fallback_temp = now_temp;
    if (std::isnan(fallback_temp)) {
        fallback_temp = s_hourly.temps[2];
    }
    s_hourly.temps[2] = fallback_temp;

    if (hourly && hourly_count > 0) {
        for (size_t i = 0; i < 2; ++i) {
            size_t offset = 2 - i;
            float temp = fallback_temp;
            if (s_hourly.hourly_cursor >= offset) {
                size_t idx = s_hourly.hourly_cursor - offset;
                temp = hourly_strip_temp_from_entry(&s_hourly.hourly_cache[idx], fallback_temp);
            }
            s_hourly.temps[i] = temp;
        }
        for (size_t i = 0; i < 4; ++i) {
            size_t idx = s_hourly.hourly_cursor + i;
            if (idx < s_hourly.hourly_count) {
                s_hourly.temps[3 + i] = hourly_strip_temp_from_entry(&s_hourly.hourly_cache[idx], fallback_temp);
            } else {
                s_hourly.temps[3 + i] = fallback_temp;
            }
        }
    } else {
        for (size_t i = 0; i < HOURLY_STRIP_ICON_COUNT; ++i) {
            s_hourly.temps[i] = fallback_temp;
        }
    }

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
        hourly_strip_detail_chart_refresh();
    }
#ifdef HOURLY_STRIP_SIMULATION
    hourly_strip_sim_tick(timeinfo, details_active);
    return;
#endif
    if (s_hourly.last_hour == timeinfo->tm_hour &&
        s_hourly.last_yday == timeinfo->tm_yday) {
        return;
    }

    s_hourly.last_hour = timeinfo->tm_hour;
    s_hourly.last_yday = timeinfo->tm_yday;

    hourly_strip_shift_no_anim();
}

extern "C" void hourly_strip_detail_chart_ensure(void)
{
    hourly_strip_detail_chart_refresh();
}
