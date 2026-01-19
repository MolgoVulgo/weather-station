#include "hourly_strip.h"

#include <string.h>
#include <time.h>
#include <lvgl.h>
#include "weather_fetcher.h"
#include "esp_log.h"
#include "weather_icons.h"
#include "ui/screens.h"
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
    HourlyIcon now_icon = {};
    lv_obj_t *ghost_icon = NULL;
#ifdef HOURLY_STRIP_SIMULATION
    int last_sim_stamp = -1;
    bool sim_pending = false;
    HourlyIcon sim_icon = {};
#endif
} HourlyStripState;

static HourlyStripState s_hourly = {};

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

static void hourly_strip_set_x(void *var, int32_t value)
{
    if (!var) {
        return;
    }
    lv_obj_set_x(static_cast<lv_obj_t *>(var), static_cast<lv_coord_t>(value));
}

static void hourly_strip_apply_all(void)
{
    if (!hourly_strip_ready()) {
        return;
    }
    for (size_t i = 0; i < HOURLY_STRIP_ICON_COUNT; ++i) {
        hourly_strip_apply_icon(hourly_strip_icon_obj(i), &s_hourly.icons[i]);
    }
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
    HourlyIcon fallback = s_hourly.now_icon.valid ? s_hourly.now_icon : s_hourly.icons[3];
    size_t next_index = s_hourly.hourly_cursor + 3;
    if (next_index < s_hourly.hourly_count) {
        return hourly_icon_from_entry(&s_hourly.hourly_cache[next_index], &fallback);
    }
    return fallback;
}

static void hourly_strip_shift_state(const HourlyIcon *new_icon)
{
    for (size_t i = 0; i + 1 < HOURLY_STRIP_ICON_COUNT; ++i) {
        s_hourly.icons[i] = s_hourly.icons[i + 1];
    }
    if (new_icon) {
        s_hourly.icons[HOURLY_STRIP_ICON_COUNT - 1] = *new_icon;
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
    hourly_strip_shift_state(&new_icon);

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
    hourly_strip_shift_state(&new_icon);
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

static void hourly_strip_sim_tick(const struct tm *timeinfo, bool details_active)
{
    if (!timeinfo || !hourly_strip_ready() || !s_hourly.initialized) {
        return;
    }

    if (!s_hourly.icons[3].valid) {
        HourlyIcon seed = hourly_strip_random_icon();
        for (size_t i = 0; i < HOURLY_STRIP_ICON_COUNT; ++i) {
            s_hourly.icons[i] = seed;
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

    if (!s_hourly.icons[3].valid && s_hourly.now_icon.valid) {
        for (size_t i = 0; i < 3; ++i) {
            s_hourly.icons[i] = s_hourly.now_icon;
        }
    }

    if (s_hourly.now_icon.valid) {
        s_hourly.icons[3] = s_hourly.now_icon;
    }

    for (size_t i = 0; i < 3; ++i) {
        size_t idx = s_hourly.hourly_cursor + i;
        HourlyIcon fallback = s_hourly.now_icon.valid ? s_hourly.now_icon : s_hourly.icons[3];
        if (idx < s_hourly.hourly_count) {
            s_hourly.icons[4 + i] = hourly_icon_from_entry(&s_hourly.hourly_cache[idx], &fallback);
        } else {
            s_hourly.icons[4 + i] = fallback;
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
