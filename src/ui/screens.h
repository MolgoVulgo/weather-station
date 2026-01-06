#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *ui_meteo;
    lv_obj_t *ui_meteo_clock;
    lv_obj_t *ui_meteo_img;
    lv_obj_t *ui_meteo_date;
    lv_obj_t *ui_meteo_temp;
    lv_obj_t *ui_meteo_condition;
    lv_obj_t *obj0;
    lv_obj_t *ui_meteo_fi_1;
    lv_obj_t *ui_meteo_fi_2;
    lv_obj_t *ui_meteo_fi_3;
    lv_obj_t *ui_meteo_fi_4;
    lv_obj_t *ui_meteo_fi_5;
    lv_obj_t *ui_meteo_fi_6;
    lv_obj_t *ui_meteo_ft1_1;
    lv_obj_t *ui_meteo_ft2_1;
    lv_obj_t *ui_meteo_ft1_2;
    lv_obj_t *ui_meteo_ft2_2;
    lv_obj_t *ui_meteo_ft1_3;
    lv_obj_t *ui_meteo_ft2_3;
    lv_obj_t *ui_meteo_ft1_4;
    lv_obj_t *ui_meteo_ft2_4;
    lv_obj_t *ui_meteo_ft1_5;
    lv_obj_t *ui_meteo_ft2_5;
    lv_obj_t *ui_meteo_ft1_6;
    lv_obj_t *ui_meteo_ft2_6;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_UI_METEO = 1,
};

void create_screen_ui_meteo();
void tick_screen_ui_meteo();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/