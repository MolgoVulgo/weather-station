#include <string.h>
#include "vars.h"

char ui_meteo_date[100] = { 0 };
char ui_meteo_houre[32] = { 0 };
char ui_meteo_temp[32] = { 0 };

const char *get_var_ui_meteo_date(void)
{
    return ui_meteo_date;
}

void set_var_ui_meteo_date(const char *value)
{
    strncpy(ui_meteo_date, value, sizeof(ui_meteo_date) / sizeof(char));
    ui_meteo_date[sizeof(ui_meteo_date) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_houre(void)
{
    return ui_meteo_houre;
}

void set_var_ui_meteo_houre(const char *value)
{
    strncpy(ui_meteo_houre, value, sizeof(ui_meteo_houre) / sizeof(char));
    ui_meteo_houre[sizeof(ui_meteo_houre) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_temp(void)
{
    return ui_meteo_temp;
}

void set_var_ui_meteo_temp(const char *value)
{
    strncpy(ui_meteo_temp, value, sizeof(ui_meteo_temp) / sizeof(char));
    ui_meteo_temp[sizeof(ui_meteo_temp) / sizeof(char) - 1] = 0;
}
