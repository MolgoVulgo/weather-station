#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>
#if LV_USE_QRCODE
#include "extra/libs/qrcode/lv_qrcode.h"
#endif

#ifndef _
#define _(s) (s)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *ui_start;
    lv_obj_t *ui_meteo;
    lv_obj_t *ui_setup;
    lv_obj_t *ui_wifi;
    lv_obj_t *ui_start_bar;
    lv_obj_t *ui_start_bar_texte;
    lv_obj_t *obj0;
    lv_obj_t *ui_meteo_clock;
    lv_obj_t *ui_meteo_img;
    lv_obj_t *ui_meteo_date;
    lv_obj_t *ui_meteo_temp;
    lv_obj_t *ui_meteo_condition;
    lv_obj_t *obj1;
    lv_obj_t *ui_meteo_fi1;
    lv_obj_t *ui_meteo_fi2;
    lv_obj_t *ui_meteo_fi3;
    lv_obj_t *ui_meteo_fi4;
    lv_obj_t *ui_meteo_fi5;
    lv_obj_t *ui_meteo_fi6;
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
    lv_obj_t *obj2;
    lv_obj_t *ui_meteo_fd1;
    lv_obj_t *ui_meteo_fd2;
    lv_obj_t *ui_meteo_fd3;
    lv_obj_t *ui_meteo_fd4;
    lv_obj_t *ui_meteo_fd5;
    lv_obj_t *ui_meteo_fd6;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_UI_START = 1,
    SCREEN_ID_UI_METEO = 2,
    SCREEN_ID_UI_SETUP = 3,
    SCREEN_ID_UI_WIFI = 4,
};

void create_screen_ui_start();
void tick_screen_ui_start();

void create_screen_ui_meteo();
void tick_screen_ui_meteo();

void create_screen_ui_setup();
void tick_screen_ui_setup();

void create_screen_ui_wifi();
void tick_screen_ui_wifi();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/
