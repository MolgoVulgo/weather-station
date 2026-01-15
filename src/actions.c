#include "actions.h"
#include "screens.h"
#include "temp_unit.h"
#include "ui.h"
#include "vars.h"
#include "weather_service.h"

static lv_obj_t *screen_from_id(enum ScreensEnum target)
{
    switch (target) {
        case SCREEN_ID_UI_START:
            return objects.ui_start;
        case SCREEN_ID_UI_METEO:
            return objects.ui_meteo;
        case SCREEN_ID_UI_METEO_DETAILS:
            return objects.ui_meteo_details;
        case SCREEN_ID_UI_SETTING:
            return objects.ui_setting;
        case SCREEN_ID_UI_WIFI:
            return objects.ui_wifi;
        default:
            return NULL;
    }
}

static void action_swipe_to(enum ScreensEnum target, lv_scr_load_anim_t anim)
{
    lv_obj_t *target_obj = screen_from_id(target);
    if (target_obj) {
        lv_scr_load_anim(target_obj, anim, 200, 0, false);
        tick_screen_by_id(target);
    }
}

void action_checked(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    bool is_checked = lv_obj_has_state(obj, LV_STATE_CHECKED);

    set_var_ui_setting_hour(is_checked);
}

void action_go_setting(lv_event_t *e)
{
    (void)e;
    loadScreen(SCREEN_ID_UI_SETTING);
    tick_screen_by_id(SCREEN_ID_UI_SETTING);
}

void action_check_unit(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    bool is_checked = lv_obj_has_state(obj, LV_STATE_CHECKED);

    set_var_ui_setting_temp(is_checked);
    tick_screen_by_id(SCREEN_ID_UI_SETTING);
    weather_service_refresh_forecast_units();
}

void action_go_ui_meteo(lv_event_t *e)
{
    (void)e;
    loadScreen(SCREEN_ID_UI_METEO);
    tick_screen_by_id(SCREEN_ID_UI_METEO);
}

void action_ui_swipe(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    lv_obj_t *target = lv_event_get_target(e);
    if (target == objects.ui_meteo && dir == LV_DIR_LEFT) {
        action_swipe_to(SCREEN_ID_UI_METEO_DETAILS, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    } else if (target == objects.ui_meteo_details && dir == LV_DIR_RIGHT) {
        action_swipe_to(SCREEN_ID_UI_METEO, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    }
}
