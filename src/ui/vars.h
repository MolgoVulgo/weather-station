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
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT1_1 = 10,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT1_2 = 11,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT1_3 = 12,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT1_4 = 13,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT1_5 = 14,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT1_6 = 15,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT2_1 = 16,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT2_2 = 17,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT2_3 = 18,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT2_4 = 19,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT2_5 = 20,
    FLOW_GLOBAL_VARIABLE_UI_METEO_FT2_6 = 21
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
extern const char *get_var_ui_meteo_ft1_1();
extern void set_var_ui_meteo_ft1_1(const char *value);
extern const char *get_var_ui_meteo_ft1_2();
extern void set_var_ui_meteo_ft1_2(const char *value);
extern const char *get_var_ui_meteo_ft1_3();
extern void set_var_ui_meteo_ft1_3(const char *value);
extern const char *get_var_ui_meteo_ft1_4();
extern void set_var_ui_meteo_ft1_4(const char *value);
extern const char *get_var_ui_meteo_ft1_5();
extern void set_var_ui_meteo_ft1_5(const char *value);
extern const char *get_var_ui_meteo_ft1_6();
extern void set_var_ui_meteo_ft1_6(const char *value);
extern const char *get_var_ui_meteo_ft2_1();
extern void set_var_ui_meteo_ft2_1(const char *value);
extern const char *get_var_ui_meteo_ft2_2();
extern void set_var_ui_meteo_ft2_2(const char *value);
extern const char *get_var_ui_meteo_ft2_3();
extern void set_var_ui_meteo_ft2_3(const char *value);
extern const char *get_var_ui_meteo_ft2_4();
extern void set_var_ui_meteo_ft2_4(const char *value);
extern const char *get_var_ui_meteo_ft2_5();
extern void set_var_ui_meteo_ft2_5(const char *value);
extern const char *get_var_ui_meteo_ft2_6();
extern void set_var_ui_meteo_ft2_6(const char *value);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/