#include "actions.h"
#include "screens.h"
#include "temp_unit.h"
#include "ui.h"
#include "vars.h"
#include "weather_service.h"

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
