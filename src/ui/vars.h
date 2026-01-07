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
    FLOW_GLOBAL_VARIABLE_UI_METEO_CONDITION = 3
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


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/