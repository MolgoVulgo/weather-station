#include "boot_progress.h"

#include <string.h>
#include <lvgl.h>
#include "esp_bsp.h"
#include "esp_log.h"
#include "ui.h"
#include "ui/screens.h"
#include "vars.h"
#include "lv_i18n.h"

static const char *TAG = "BootProgress";

static void boot_progress_load_screen(lv_obj_t *screen)
{
    if (!screen) {
        return;
    }
    bsp_display_lock(0);
    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
    bsp_display_unlock();
}

void boot_progress_init(void)
{
    set_var_ui_start_bar(0);
    set_var_ui_start_bar_texte(_("Demarrage..."));
    boot_progress_load_screen(objects.ui_start);
}

void boot_progress_set(int32_t value, const char *text)
{
    set_var_ui_start_bar(value);
    if (text && text[0] != '\0') {
        set_var_ui_start_bar_texte(text);
    }
    if (!objects.ui_start_bar || !objects.ui_start_bar_texte) {
        ESP_LOGW(TAG, "UI start non prete, boot_progress_set ignore");
        return;
    }
    if (!bsp_display_lock(0)) {
        ESP_LOGW(TAG, "UI lock echoue, boot_progress_set ignore");
        return;
    }
    tick_screen_by_id(SCREEN_ID_UI_START);
    bsp_display_unlock();
}

void boot_progress_show_meteo(void)
{
    if (!objects.ui_meteo) {
        ESP_LOGW(TAG, "UI meteo non prete, boot_progress_show_meteo ignore");
        return;
    }
    if (!bsp_display_lock(0)) {
        ESP_LOGW(TAG, "UI lock echoue, boot_progress_show_meteo ignore");
        return;
    }
    loadScreen(SCREEN_ID_UI_METEO);
    tick_screen_by_id(SCREEN_ID_UI_METEO);
    bsp_display_unlock();
}

void boot_progress_show_wifi(void)
{
    if (!objects.ui_wifi) {
        ESP_LOGW(TAG, "UI wifi non prete, boot_progress_show_wifi ignore");
        return;
    }
    if (!bsp_display_lock(0)) {
        ESP_LOGW(TAG, "UI lock echoue, boot_progress_show_wifi ignore");
        return;
    }
    loadScreen(SCREEN_ID_UI_WIFI);
    tick_screen_by_id(SCREEN_ID_UI_WIFI);
    bsp_display_unlock();
}
