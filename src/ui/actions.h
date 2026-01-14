#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_checked(lv_event_t * e);
extern void action_go_setting(lv_event_t * e);
extern void action_check_unit(lv_event_t * e);
extern void action_go_ui_meteo(lv_event_t * e);
extern void action_ui_swipe(lv_event_t * e);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/