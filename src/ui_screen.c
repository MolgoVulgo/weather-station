#include "ui_screen.h"

#include <lvgl.h>
#include <inttypes.h>
#include "ui_backend.h"
#include "time_sync.h"

static uint32_t clock_seconds;

static void ui_screen_apply_time(lv_obj_t *clock, const struct tm *timeinfo)
{
    if (!clock || !timeinfo) {
        return;
    }
    lv_label_set_text_fmt(clock, "%02d:%02d:%02d",
                          timeinfo->tm_hour,
                          timeinfo->tm_min,
                          timeinfo->tm_sec);
    clock_seconds = (uint32_t)(timeinfo->tm_hour * 3600 +
                               timeinfo->tm_min * 60 +
                               timeinfo->tm_sec);
}

static void ui_screen_tick(lv_timer_t *timer)
{
    (void)timer;
    lv_obj_t *clock = ui_time_label();
    if (!clock) {
        return;
    }

    struct tm timeinfo;
    if (time_sync_get_local_time(&timeinfo, NULL)) {
        ui_screen_apply_time(clock, &timeinfo);
        return;
    }

    clock_seconds = (clock_seconds + 1) % (24U * 60U * 60U);
    uint32_t hours = clock_seconds / 3600U;
    uint32_t minutes = (clock_seconds / 60U) % 60U;
    uint32_t seconds = clock_seconds % 60U;
    lv_label_set_text_fmt(clock, "%02" PRIu32 ":%02" PRIu32 ":%02" PRIu32, hours, minutes, seconds);
}

void ui_screen_start(void)
{
    lv_obj_t *clock = ui_time_label();
    if (clock) {
        struct tm timeinfo;
        if (time_sync_get_local_time(&timeinfo, NULL)) {
            ui_screen_apply_time(clock, &timeinfo);
        } else {
            clock_seconds = 0;
            lv_label_set_text(clock, "00:00:00");
        }
    }
    lv_timer_create(ui_screen_tick, 1000, NULL);
}
