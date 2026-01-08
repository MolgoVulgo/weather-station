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
    lv_obj_t *obj1;
    lv_obj_t *ui_meteo_fd1;
    lv_obj_t *ui_meteo_fd2;
    lv_obj_t *ui_meteo_fd3;
    lv_obj_t *ui_meteo_fd4;
    lv_obj_t *ui_meteo_fd5;
    lv_obj_t *ui_meteo_fd6;
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