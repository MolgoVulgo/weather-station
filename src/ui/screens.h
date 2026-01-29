#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _groups_t {
    lv_group_t *test;
} groups_t;

extern groups_t groups;

void ui_create_groups();

typedef struct _objects_t {
    lv_obj_t *ui_start;
    lv_obj_t *ui_meteo;
    lv_obj_t *ui_meteo_details;
    lv_obj_t *ui_setting;
    lv_obj_t *ui_wifi;
    lv_obj_t *ui_setting_hour;
    lv_obj_t *ui_setting_unit;
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
    lv_obj_t *obj2;
    lv_obj_t *ui_meteo_fd1;
    lv_obj_t *ui_meteo_fd2;
    lv_obj_t *ui_meteo_fd3;
    lv_obj_t *ui_meteo_fd4;
    lv_obj_t *ui_meteo_fd5;
    lv_obj_t *ui_meteo_fd6;
    lv_obj_t *ui_meteo_setting;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *ui_meteo_ft1_2;
    lv_obj_t *ui_meteo_ft1_3;
    lv_obj_t *ui_meteo_ft1_4;
    lv_obj_t *ui_meteo_ft1_5;
    lv_obj_t *ui_meteo_ft1_6;
    lv_obj_t *obj8;
    lv_obj_t *aler_meteo;
    lv_obj_t *obj9;
    lv_obj_t *ui_detail_hourly;
    lv_obj_t *obj10;
    lv_obj_t *obj10__obj0;
    lv_obj_t *obj11;
    lv_obj_t *obj11__obj0;
    lv_obj_t *obj12;
    lv_obj_t *obj12__obj0;
    lv_obj_t *obj13;
    lv_obj_t *obj13__obj0;
    lv_obj_t *obj14;
    lv_obj_t *obj14__obj0;
    lv_obj_t *obj15;
    lv_obj_t *obj15__obj0;
    lv_obj_t *obj16;
    lv_obj_t *obj16__obj0;
    lv_obj_t *ui_detail_chart;
    lv_obj_t *obj17;
    lv_obj_t *obj18;
    lv_obj_t *obj19;
    lv_obj_t *obj20;
    lv_obj_t *ui_humidity;
    lv_obj_t *ui_pop;
    lv_obj_t *ui_clouds;
    lv_obj_t *obj21;
    lv_obj_t *obj22;
    lv_obj_t *obj23;
    lv_obj_t *ui_setting_gmt_switch;
    lv_obj_t *ui_setting_gmt_label;
    lv_obj_t *ui_setting_gmt_switch1;
    lv_obj_t *ui_setting_gmt_switch2;
    lv_obj_t *obj24;
    lv_obj_t *obj25;
    lv_obj_t *obj26;
    lv_obj_t *obj27;
    lv_obj_t *ui_setting_laguage;
    lv_obj_t *ui_settings_save;
    lv_obj_t *obj28;
    lv_obj_t *obj29;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_UI_START = 1,
    SCREEN_ID_UI_METEO = 2,
    SCREEN_ID_UI_METEO_DETAILS = 3,
    SCREEN_ID_UI_SETTING = 4,
    SCREEN_ID_UI_WIFI = 5,
};

void create_screen_ui_start();
void tick_screen_ui_start();

void create_screen_ui_meteo();
void tick_screen_ui_meteo();

void create_screen_ui_meteo_details();
void tick_screen_ui_meteo_details();

void create_screen_ui_setting();
void tick_screen_ui_setting();

void create_screen_ui_wifi();
void tick_screen_ui_wifi();

void create_user_widget_hourly(lv_obj_t *parent_obj, int startWidgetIndex);
void tick_user_widget_hourly(int startWidgetIndex);

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/