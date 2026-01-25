#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations



// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_UI_METEO_DATE = 0,
    FLOW_GLOBAL_VARIABLE_UI_METEO_HOURE = 1,
    FLOW_GLOBAL_VARIABLE_UI_METEO_TEMP = 2,
    FLOW_GLOBAL_VARIABLE_UI_METEO_CONDITION = 3,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FD1 = 4,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FD2 = 5,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FD3 = 6,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FD4 = 7,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FD5 = 8,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FD6 = 9,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT1 = 10,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT2 = 11,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT3 = 12,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT4 = 13,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT5 = 14,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT6 = 15,
    FLOW_GLOBAL_VARIABLE_UI_START_BAR = 16,
    FLOW_GLOBAL_VARIABLE_UI_START_BAR_TEXTE = 17,
    FLOW_GLOBAL_VARIABLE_UI_SETTING_GMT = 18,
    FLOW_GLOBAL_VARIABLE_UI_SETTING_HOUR = 19,
    FLOW_GLOBAL_VARIABLE_UI_SETTING_GMT_TXT = 20,
    FLOW_GLOBAL_VARIABLE_UI_SETTING_TEMP = 21,
    FLOW_GLOBAL_VARIABLE_UI_SETTING_LAGUAGE = 22,
    FLOW_GLOBAL_VARIABLE_UI_HUMIDITY = 23,
    FLOW_GLOBAL_VARIABLE_UI_CLOUDS = 24,
    FLOW_GLOBAL_VARIABLE_UI_POP = 25
};

// Native global variables

extern const char *get_var_ui_meteo_date();
extern void set_var_ui_meteo_date(const char *value);
extern const char *get_var_ui_meteo_houre();
extern void set_var_ui_meteo_houre(const char *value);
extern const char *get_var_ui_meteo_temp();
extern void set_var_ui_meteo_temp(const char *value);
extern const char *get_var_ui_meteo_condition();
extern void set_var_ui_meteo_condition(const char *value);
extern const char *get_var_ui_meteo_fd1();
extern void set_var_ui_meteo_fd1(const char *value);
extern const char *get_var_ui_meteo_fd2();
extern void set_var_ui_meteo_fd2(const char *value);
extern const char *get_var_ui_meteo_fd3();
extern void set_var_ui_meteo_fd3(const char *value);
extern const char *get_var_ui_meteo_fd4();
extern void set_var_ui_meteo_fd4(const char *value);
extern const char *get_var_ui_meteo_fd5();
extern void set_var_ui_meteo_fd5(const char *value);
extern const char *get_var_ui_meteo_fd6();
extern void set_var_ui_meteo_fd6(const char *value);
extern const char *get_var_ui_meteo_ft1();
extern void set_var_ui_meteo_ft1(const char *value);
extern const char *get_var_ui_meteo_ft2();
extern void set_var_ui_meteo_ft2(const char *value);
extern const char *get_var_ui_meteo_ft3();
extern void set_var_ui_meteo_ft3(const char *value);
extern const char *get_var_ui_meteo_ft4();
extern void set_var_ui_meteo_ft4(const char *value);
extern const char *get_var_ui_meteo_ft5();
extern void set_var_ui_meteo_ft5(const char *value);
extern const char *get_var_ui_meteo_ft6();
extern void set_var_ui_meteo_ft6(const char *value);
extern int32_t get_var_ui_start_bar();
extern void set_var_ui_start_bar(int32_t value);
extern const char *get_var_ui_start_bar_texte();
extern void set_var_ui_start_bar_texte(const char *value);
extern int32_t get_var_ui_setting_gmt();
extern void set_var_ui_setting_gmt(int32_t value);
extern bool get_var_ui_setting_hour();
extern void set_var_ui_setting_hour(bool value);
extern const char *get_var_ui_setting_gmt_txt();
extern void set_var_ui_setting_gmt_txt(const char *value);
extern bool get_var_ui_setting_temp();
extern void set_var_ui_setting_temp(bool value);
extern int32_t get_var_ui_setting_laguage();
extern void set_var_ui_setting_laguage(int32_t value);
extern const char *get_var_ui_humidity();
extern void set_var_ui_humidity(const char *value);
extern const char *get_var_ui_clouds();
extern void set_var_ui_clouds(const char *value);
extern const char *get_var_ui_pop();
extern void set_var_ui_pop(const char *value);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/