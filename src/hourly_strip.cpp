#include "hourly_strip.h"

#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <cmath>
#include <lvgl.h>
#include "weather_fetcher.h"
#include "esp_log.h"
#include "weather_icons.h"
#include "ui/screens.h"
#include "vars.h"
#include "temp_unit.h"
#ifdef HOURLY_STRIP_SIMULATION
#include "esp_system.h"
#include "esp_random.h"
#endif

#define HOURLY_STRIP_ICON_COUNT 7
#define HOURLY_CACHE_MAX 12

typedef struct {
    uint16_t condition_id = 0;
    uint8_t variant = 0;
    bool valid = false;
} HourlyIcon;

typedef struct {
    bool initialized = false;
    bool animating = false;
    int16_t base_x = 0;
    int16_t icon_step = 0;
    int last_hour = -1;
    int last_yday = -1;
    size_t hourly_count = 0;
    size_t hourly_cursor = 0;
    HourlyEntry hourly_cache[HOURLY_CACHE_MAX] = {};
    HourlyIcon icons[HOURLY_STRIP_ICON_COUNT] = {};
    float temps[HOURLY_STRIP_ICON_COUNT] = {};
    HourlyIcon now_icon = {};
    lv_obj_t *ghost_icon = NULL;
#ifdef HOURLY_STRIP_SIMULATION
    int last_sim_stamp = -1;
    bool sim_pending = false;
    HourlyIcon sim_icon = {};
    float sim_temp = NAN;
#endif
} HourlyStripState;

static HourlyStripState s_hourly = {};
static lv_chart_series_t *s_hourly_chart_series = NULL;
static lv_obj_t *s_hourly_chart_line_top = NULL;
static lv_obj_t *s_hourly_chart_line_mid = NULL;
static lv_obj_t *s_hourly_chart_line_bottom = NULL;
static lv_obj_t *s_hourly_chart_label_top = NULL;
static lv_obj_t *s_hourly_chart_label_mid = NULL;
static lv_obj_t *s_hourly_chart_label_bottom = NULL;
static lv_point_t s_hourly_chart_line_points_top[2];
static lv_point_t s_hourly_chart_line_points_mid[2];
static lv_point_t s_hourly_chart_line_points_bottom[2];

static lv_obj_t *hourly_strip_icon_obj(size_t index)
{
    switch (index) {
    case 0:
        return objects.hourly_icon_0;
    case 1:
        return objects.hourly_icon_1;
    case 2:
        return objects.hourly_icon_2;
    case 3:
        return objects.hourly_icon_3;
    case 4:
        return objects.hourly_icon_4;
    case 5:
        return objects.hourly_icon_5;
    case 6:
        return objects.hourly_icon_6;
    default:
        return NULL;
    }
}

static bool hourly_strip_ready(void)
{
    return objects.hourly_strip &&
           objects.hourly_icon_0 &&
           objects.hourly_icon_6;
}

static HourlyIcon hourly_icon_from_current(const CurrentWeatherData *current)
{
    HourlyIcon icon;
    if (!current) {
        return icon;
    }
    if (current->conditionId != 0) {
        icon.condition_id = static_cast<uint16_t>(current->conditionId);
        icon.variant = current->iconVariant;
        icon.valid = true;
    }
    return icon;
}

static HourlyIcon hourly_icon_from_entry(const HourlyEntry *entry, const HourlyIcon *fallback)
{
    if (entry && entry->valid && entry->conditionId != 0) {
        HourlyIcon icon;
        icon.condition_id = static_cast<uint16_t>(entry->conditionId);
        icon.variant = entry->iconVariant;
        icon.valid = true;
        return icon;
    }
    return (fallback != NULL) ? *fallback : HourlyIcon();
}

static void hourly_strip_apply_icon(lv_obj_t *target, const HourlyIcon *icon)
{
    if (!target || !icon || !icon->valid) {
        return;
    }
    weather_icons_set_object(target, "icon_50.bin", icon->condition_id, icon->variant);
}

static void hourly_strip_init_chart(void)
{
    if (!objects.hoyly_char || s_hourly_chart_series) {
        return;
    }

    lv_chart_set_type(objects.hoyly_char, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(objects.hoyly_char, HOURLY_STRIP_ICON_COUNT);
    lv_chart_set_update_mode(objects.hoyly_char, LV_CHART_UPDATE_MODE_SHIFT);
    lv_obj_set_style_pad_left(objects.hoyly_char, 25, LV_PART_MAIN);
    lv_obj_set_style_pad_right(objects.hoyly_char, 25, LV_PART_MAIN);
    s_hourly_chart_series = lv_chart_add_series(
        objects.hoyly_char,
        lv_color_hex(0xffffffff),
        LV_CHART_AXIS_PRIMARY_Y);

    s_hourly_chart_line_top = lv_line_create(objects.hoyly_char);
    s_hourly_chart_line_mid = lv_line_create(objects.hoyly_char);
    s_hourly_chart_line_bottom = lv_line_create(objects.hoyly_char);
    lv_obj_set_style_line_width(s_hourly_chart_line_top, 1, LV_PART_MAIN);
    lv_obj_set_style_line_width(s_hourly_chart_line_mid, 1, LV_PART_MAIN);
    lv_obj_set_style_line_width(s_hourly_chart_line_bottom, 1, LV_PART_MAIN);
    lv_obj_set_style_line_color(s_hourly_chart_line_top, lv_color_hex(0xffffffff), LV_PART_MAIN);
    lv_obj_set_style_line_color(s_hourly_chart_line_mid, lv_color_hex(0xffffffff), LV_PART_MAIN);
    lv_obj_set_style_line_color(s_hourly_chart_line_bottom, lv_color_hex(0xffffffff), LV_PART_MAIN);

    s_hourly_chart_label_top = lv_label_create(objects.hoyly_char);
    s_hourly_chart_label_mid = lv_label_create(objects.hoyly_char);
    s_hourly_chart_label_bottom = lv_label_create(objects.hoyly_char);
    lv_obj_set_style_text_color(s_hourly_chart_label_top, lv_color_hex(0xffffffff), LV_PART_MAIN);
    lv_obj_set_style_text_color(s_hourly_chart_label_mid, lv_color_hex(0xffffffff), LV_PART_MAIN);
    lv_obj_set_style_text_color(s_hourly_chart_label_bottom, lv_color_hex(0xffffffff), LV_PART_MAIN);
}

static float hourly_strip_to_unit(float temp_c, bool is_fahrenheit)
{
    if (std::isnan(temp_c)) {
        return temp_c;
    }
    if (!is_fahrenheit) {
        return temp_c;
    }
    return (temp_c * 9.0f / 5.0f) + 32.0f;
}

static float hourly_strip_round_step(float value, float step)
{
    if (std::isnan(value) || step <= 0.0f) {
        return value;
    }
    return step * std::round(value / step);
}

static void hourly_strip_chart_set_line(lv_obj_t *line,
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

static void hourly_strip_chart_set_label(lv_obj_t *label, const char *text, lv_coord_t x, lv_coord_t y)
{
    if (!label) {
        return;
    }
    lv_label_set_text(label, text ? text : "");
    lv_obj_update_layout(label);
    lv_coord_t h = lv_obj_get_height(label);
    lv_obj_set_pos(label, x, (lv_coord_t)(y - h / 2));
}

static void hourly_strip_update_chart(void)
{
    hourly_strip_init_chart();
    if (!objects.hoyly_char || !s_hourly_chart_series) {
        return;
    }

    bool is_fahrenheit = temp_unit_is_fahrenheit();
    float step = is_fahrenheit ? 50.0f : 10.0f;
    bool any_valid = false;
    float min_val = 0.0f;
    float max_val = 0.0f;
    for (size_t i = 0; i < HOURLY_STRIP_ICON_COUNT; ++i) {
        float temp = hourly_strip_to_unit(s_hourly.temps[i], is_fahrenheit);
        if (!std::isnan(temp)) {
            if (!any_valid) {
                min_val = temp;
                max_val = temp;
                any_valid = true;
            } else {
                if (temp < min_val) {
                    min_val = temp;
                }
                if (temp > max_val) {
                    max_val = temp;
                }
            }
        }
    }

    float fallback = any_valid ? min_val : 0.0f;
    for (size_t i = 0; i < HOURLY_STRIP_ICON_COUNT; ++i) {
        float temp = hourly_strip_to_unit(s_hourly.temps[i], is_fahrenheit);
        if (std::isnan(temp)) {
            temp = fallback;
        }
        lv_chart_set_value_by_id(objects.hoyly_char,
                                 s_hourly_chart_series,
                                 (uint16_t)i,
                                 (lv_coord_t)lrintf(temp));
    }

    float mid_val = 0.0f;
    bool mid_is_zero = false;
    if (!any_valid) {
        mid_val = 0.0f;
    } else if (min_val >= 0.0f) {
        mid_val = hourly_strip_round_step((min_val + max_val) / 2.0f, step);
    } else if (max_val <= 0.0f) {
        mid_val = hourly_strip_round_step((min_val + max_val) / 2.0f, step);
    } else if (min_val >= -step && max_val <= step) {
        mid_val = 0.0f;
        mid_is_zero = true;
    } else {
        mid_val = hourly_strip_round_step((min_val + max_val) / 2.0f, step);
    }

    float top_val = mid_val + step;
    float bottom_val = mid_val - step;
    lv_chart_set_range(objects.hoyly_char,
                       LV_CHART_AXIS_PRIMARY_Y,
                       (int32_t)lrintf(bottom_val),
                       (int32_t)lrintf(top_val));

    lv_obj_update_layout(objects.hoyly_char);
    lv_coord_t chart_w = lv_obj_get_width(objects.hoyly_char);
    lv_coord_t chart_h = lv_obj_get_height(objects.hoyly_char);
    lv_coord_t pad_left = lv_obj_get_style_pad_left(objects.hoyly_char, LV_PART_MAIN);
    lv_coord_t pad_right = lv_obj_get_style_pad_right(objects.hoyly_char, LV_PART_MAIN);
    lv_coord_t plot_w = chart_w - pad_left - pad_right;
    if (plot_w < 0) {
        plot_w = 0;
    }
    lv_coord_t y_top = 0;
    lv_coord_t y_mid = chart_h / 2;
    lv_coord_t y_bottom = chart_h - 1;

    hourly_strip_chart_set_line(s_hourly_chart_line_top,
                                s_hourly_chart_line_points_top,
                                pad_left,
                                y_top,
                                plot_w);
    hourly_strip_chart_set_line(s_hourly_chart_line_mid,
                                s_hourly_chart_line_points_mid,
                                pad_left,
                                y_mid,
                                plot_w);
    hourly_strip_chart_set_line(s_hourly_chart_line_bottom,
                                s_hourly_chart_line_points_bottom,
                                pad_left,
                                y_bottom,
                                plot_w);

    lv_color_t mid_color = mid_is_zero ? lv_color_hex(0xff00aaff) : lv_color_hex(0xffffffff);
    lv_obj_set_style_line_color(s_hourly_chart_line_mid, mid_color, LV_PART_MAIN);

    char label_buf[16];
    snprintf(label_buf, sizeof(label_buf), "%.0f", top_val);
    hourly_strip_chart_set_label(s_hourly_chart_label_top, label_buf, 0, y_top);
    snprintf(label_buf, sizeof(label_buf), "%.0f", mid_val);
    hourly_strip_chart_set_label(s_hourly_chart_label_mid, label_buf, 0, y_mid);
    snprintf(label_buf, sizeof(label_buf), "%.0f", bottom_val);
    hourly_strip_chart_set_label(s_hourly_chart_label_bottom, label_buf, 0, y_bottom);

    lv_chart_refresh(objects.hoyly_char);
}

static float hourly_strip_temp_from_entry(const HourlyEntry *entry, float fallback)
{
    if (entry && entry->valid && !std::isnan(entry->temperature)) {
        return entry->temperature;
    }
    return fallback;
}

static void hourly_strip_format_temp(float value, char *out, size_t out_size)
{
    if (out_size == 0) {
        return;
    }
    if (std::isnan(value)) {
        out[0] = '\0';
        return;
    }
    snprintf(out, out_size, "%.0f", value);
    out[out_size - 1] = '\0';
}

static void hourly_strip_set_temp_var(size_t index, float value)
{
    char temp_buf[16];
    hourly_strip_format_temp(value, temp_buf, sizeof(temp_buf));
    switch (index) {
    case 0:
        set_var_hourly_temp_0((int32_t)(intptr_t)temp_buf);
        break;
    case 1:
        set_var_hourly_temp_1((int32_t)(intptr_t)temp_buf);
        break;
    case 2:
        set_var_hourly_temp_2((int32_t)(intptr_t)temp_buf);
        break;
    case 3:
        set_var_hourly_temp_3((int32_t)(intptr_t)temp_buf);
        break;
    case 4:
        set_var_hourly_temp_4((int32_t)(intptr_t)temp_buf);
        break;
    case 5:
        set_var_hourly_temp_5((int32_t)(intptr_t)temp_buf);
        break;
    case 6:
        set_var_hourly_temp_6((int32_t)(intptr_t)temp_buf);
        break;
    default:
        break;
    }
}

static void hourly_strip_apply_temps(void)
{
    for (size_t i = 0; i < HOURLY_STRIP_ICON_COUNT; ++i) {
        hourly_strip_set_temp_var(i, s_hourly.temps[i]);
    }
}

static void hourly_strip_set_x(void *var, int32_t value)
{
    if (!var) {
        return;
    }
    lv_obj_set_x(static_cast<lv_obj_t *>(var), static_cast<lv_coord_t>(value));
}

static void hourly_strip_apply_all(void)
{
    if (hourly_strip_ready()) {
        for (size_t i = 0; i < HOURLY_STRIP_ICON_COUNT; ++i) {
            hourly_strip_apply_icon(hourly_strip_icon_obj(i), &s_hourly.icons[i]);
        }
    }
    hourly_strip_apply_temps();
    hourly_strip_update_chart();
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

static HourlyIcon hourly_strip_next_icon(void)
{
#ifdef HOURLY_STRIP_SIMULATION
    if (s_hourly.sim_pending) {
        return s_hourly.sim_icon;
    }
#endif
    HourlyIcon fallback = s_hourly.now_icon.valid ? s_hourly.now_icon : s_hourly.icons[2];
    size_t next_index = s_hourly.hourly_cursor + 4;
    if (next_index < s_hourly.hourly_count) {
        return hourly_icon_from_entry(&s_hourly.hourly_cache[next_index], &fallback);
    }
    return fallback;
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

static void hourly_strip_shift_state(const HourlyIcon *new_icon, const float *new_temp)
{
    for (size_t i = 0; i + 1 < HOURLY_STRIP_ICON_COUNT; ++i) {
        s_hourly.icons[i] = s_hourly.icons[i + 1];
    }
    if (new_icon) {
        s_hourly.icons[HOURLY_STRIP_ICON_COUNT - 1] = *new_icon;
    }
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

static void hourly_strip_anim_ready_cb(lv_anim_t *anim)
{
    (void)anim;
    if (s_hourly.ghost_icon) {
        lv_obj_del(s_hourly.ghost_icon);
        s_hourly.ghost_icon = NULL;
    }

    HourlyIcon new_icon = hourly_strip_next_icon();
    float new_temp = hourly_strip_next_temp();
    hourly_strip_shift_state(&new_icon, &new_temp);

    if (objects.hourly_strip) {
        lv_obj_set_x(objects.hourly_strip, s_hourly.base_x);
    }
    hourly_strip_apply_all();
    s_hourly.animating = false;
}

static void hourly_strip_start_anim(void)
{
    if (!hourly_strip_ready() || s_hourly.animating) {
        return;
    }

    s_hourly.base_x = lv_obj_get_x(objects.hourly_strip);
    s_hourly.animating = true;

    if (!s_hourly.ghost_icon) {
        s_hourly.ghost_icon = lv_img_create(objects.hourly_strip);
        lv_obj_set_size(s_hourly.ghost_icon, s_hourly.icon_step, s_hourly.icon_step);
        lv_obj_set_pos(s_hourly.ghost_icon, s_hourly.icon_step * HOURLY_STRIP_ICON_COUNT, 0);
    }
    HourlyIcon new_icon = hourly_strip_next_icon();
    hourly_strip_apply_icon(s_hourly.ghost_icon, &new_icon);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, objects.hourly_strip);
    lv_anim_set_exec_cb(&anim, hourly_strip_set_x);
    lv_anim_set_values(&anim, s_hourly.base_x, s_hourly.base_x - s_hourly.icon_step);
    lv_anim_set_time(&anim, 300);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_ready_cb(&anim, hourly_strip_anim_ready_cb);
    lv_anim_start(&anim);
}

static void hourly_strip_shift_no_anim(void)
{
    HourlyIcon new_icon = hourly_strip_next_icon();
    float new_temp = hourly_strip_next_temp();
    hourly_strip_shift_state(&new_icon, &new_temp);
    hourly_strip_apply_all();
}

#ifdef HOURLY_STRIP_SIMULATION
static HourlyIcon hourly_strip_random_icon(void)
{
    static const uint16_t k_codes[] = { 200, 300, 500, 600, 701, 800, 801, 802, 803, 804 };
    uint32_t r = esp_random();
    size_t idx = r % (sizeof(k_codes) / sizeof(k_codes[0]));
    HourlyIcon icon;
    icon.condition_id = k_codes[idx];
    icon.variant = (uint8_t)(r & 0x1);
    icon.valid = true;
    return icon;
}

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

    if (!s_hourly.icons[2].valid) {
        HourlyIcon seed = hourly_strip_random_icon();
        float temp_seed = hourly_strip_random_temp();
        for (size_t i = 0; i < HOURLY_STRIP_ICON_COUNT; ++i) {
            s_hourly.icons[i] = seed;
            s_hourly.temps[i] = temp_seed;
        }
        s_hourly.now_icon = seed;
        hourly_strip_apply_all();
    }

    int stamp = timeinfo->tm_min * 60 + timeinfo->tm_sec;
    if (stamp == s_hourly.last_sim_stamp) {
        return;
    }
    s_hourly.last_sim_stamp = stamp;
    if ((timeinfo->tm_sec % 30) != 0) {
        return;
    }

    s_hourly.sim_icon = hourly_strip_random_icon();
    s_hourly.sim_temp = hourly_strip_random_temp();
    s_hourly.sim_pending = true;
    if (details_active) {
        hourly_strip_start_anim();
    } else {
        hourly_strip_shift_no_anim();
    }
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
        s_hourly.base_x = lv_obj_get_x(objects.hourly_strip);
        s_hourly.icon_step = static_cast<int16_t>(lv_obj_get_width(objects.hourly_icon_0));
        if (s_hourly.icon_step <= 0) {
            s_hourly.icon_step = 50;
        }
        for (size_t i = 0; i < HOURLY_STRIP_ICON_COUNT; ++i) {
            s_hourly.temps[i] = NAN;
        }
        s_hourly.initialized = true;
    }

    HourlyIcon now_icon = hourly_icon_from_current(current);
    if (now_icon.valid) {
        s_hourly.now_icon = now_icon;
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

    if (!s_hourly.icons[2].valid && s_hourly.now_icon.valid) {
        for (size_t i = 0; i < 2; ++i) {
            s_hourly.icons[i] = s_hourly.now_icon;
        }
    }

    if (s_hourly.now_icon.valid) {
        s_hourly.icons[2] = s_hourly.now_icon;
    }

    for (size_t i = 0; i < 4; ++i) {
        size_t idx = s_hourly.hourly_cursor + i;
        HourlyIcon fallback = s_hourly.now_icon.valid ? s_hourly.now_icon : s_hourly.icons[2];
        if (idx < s_hourly.hourly_count) {
            s_hourly.icons[3 + i] = hourly_icon_from_entry(&s_hourly.hourly_cache[idx], &fallback);
        } else {
            s_hourly.icons[3 + i] = fallback;
        }
    }

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

    hourly_strip_apply_all();
}

void hourly_strip_tick(const struct tm *timeinfo, bool details_active)
{
    if (!timeinfo || !hourly_strip_ready() || !s_hourly.initialized) {
        return;
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

    if (details_active) {
        hourly_strip_start_anim();
#ifdef DEBUG_LOG
        ESP_LOGI("HOURLY", "Animation horaire details");
#endif
    } else {
        hourly_strip_shift_no_anim();
#ifdef DEBUG_LOG
        ESP_LOGI("HOURLY", "Shift horaire sans animation");
#endif
    }
}
