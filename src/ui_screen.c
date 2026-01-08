#include "ui_screen.h"

#include <lvgl.h>
#include <stdio.h>
#include <esp_log.h>
#include "time_sync.h"
#include "lanague.h"
#include "vars.h"
#include "boot_progress.h"
#include "weather_service.h"

static uint32_t clock_seconds;
static bool time_synced_once;

static void ui_screen_apply_time(const struct tm *timeinfo)
{
    if (!timeinfo) {
        return;
    }
    char buffer[32];
    int written = snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
                           timeinfo->tm_hour,
                           timeinfo->tm_min,
                           timeinfo->tm_sec);
    if (written <= 0) {
        return;
    }
    set_var_ui_meteo_houre(buffer);
    clock_seconds = (uint32_t)(timeinfo->tm_hour * 3600 +
                               timeinfo->tm_min * 60 +
                               timeinfo->tm_sec);
}

static void ui_screen_apply_date(const struct tm *timeinfo)
{
    if (!timeinfo) {
        return;
    }

    const lanague_table_t *table = lanague_get_table(lanague_get_current());
    int wday = timeinfo->tm_wday;
    int mday = timeinfo->tm_mday;
    int mon = timeinfo->tm_mon;
    if (wday < 0 || wday > 6 || mon < 0 || mon > 11) {
        return;
    }

    char buffer[64];
    int written = snprintf(buffer, sizeof(buffer), "%s %d %s",
                           table->day_names[wday],
                           mday,
                           table->month_names[mon]);
    if (written <= 0) {
        return;
    }
    set_var_ui_meteo_date(buffer);
}

static void ui_screen_tick(lv_timer_t *timer)
{
    (void)timer;
    struct tm timeinfo;
    if (time_sync_get_local_time(&timeinfo, NULL)) {
        if (!time_synced_once) {
            time_synced_once = true;
            ESP_LOGI("UI", "Heure NTP valide, mise a jour UI");
            boot_progress_set(60, "NTP OK");
            weather_service_request_update();
        }
        ui_screen_apply_time(&timeinfo);
        ui_screen_apply_date(&timeinfo);
        return;
    }

    clock_seconds = (clock_seconds + 1) % (24U * 60U * 60U);
    uint32_t hours = clock_seconds / 3600U;
    uint32_t minutes = (clock_seconds / 60U) % 60U;
    uint32_t seconds = clock_seconds % 60U;
    char buffer[32];
    int written = snprintf(buffer, sizeof(buffer), "%02u:%02u:%02u",
                           (unsigned)hours,
                           (unsigned)minutes,
                           (unsigned)seconds);
    if (written > 0) {
        set_var_ui_meteo_houre(buffer);
    }
}

void ui_screen_start(void)
{
    struct tm timeinfo;
    if (time_sync_get_local_time(&timeinfo, NULL)) {
        ui_screen_apply_time(&timeinfo);
        ui_screen_apply_date(&timeinfo);
    } else {
        clock_seconds = 0;
        set_var_ui_meteo_houre("00:00:00");
        set_var_ui_meteo_date("");
    }
    lv_timer_create(ui_screen_tick, 1000, NULL);
}
