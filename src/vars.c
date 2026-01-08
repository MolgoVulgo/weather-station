#include <string.h>
#include "vars.h"

char ui_meteo_date[100] = { 0 };
char ui_meteo_houre[32] = { 0 };
char ui_meteo_temp[32] = { 0 };
char ui_meteo_condition[64] = { 0 };
char ui_meteo_fd1[16] = { 0 };
char ui_meteo_fd2[16] = { 0 };
char ui_meteo_fd3[16] = { 0 };
char ui_meteo_fd4[16] = { 0 };
char ui_meteo_fd5[16] = { 0 };
char ui_meteo_fd6[16] = { 0 };
char ui_meteo_ft1_1[16] = { 0 };
char ui_meteo_ft1_2[16] = { 0 };
char ui_meteo_ft1_3[16] = { 0 };
char ui_meteo_ft1_4[16] = { 0 };
char ui_meteo_ft1_5[16] = { 0 };
char ui_meteo_ft1_6[16] = { 0 };
char ui_meteo_ft2_1[16] = { 0 };
char ui_meteo_ft2_2[16] = { 0 };
char ui_meteo_ft2_3[16] = { 0 };
char ui_meteo_ft2_4[16] = { 0 };
char ui_meteo_ft2_5[16] = { 0 };
char ui_meteo_ft2_6[16] = { 0 };
char ui_start_bar_texte[64] = { 0 };
int32_t ui_start_bar = 0;

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

const char *get_var_ui_meteo_condition(void)
{
    return ui_meteo_condition;
}

void set_var_ui_meteo_condition(const char *value)
{
    strncpy(ui_meteo_condition, value, sizeof(ui_meteo_condition) / sizeof(char));
    ui_meteo_condition[sizeof(ui_meteo_condition) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_fd1(void)
{
    return ui_meteo_fd1;
}

void set_var_ui_meteo_fd1(const char *value)
{
    strncpy(ui_meteo_fd1, value, sizeof(ui_meteo_fd1) / sizeof(char));
    ui_meteo_fd1[sizeof(ui_meteo_fd1) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_fd2(void)
{
    return ui_meteo_fd2;
}

void set_var_ui_meteo_fd2(const char *value)
{
    strncpy(ui_meteo_fd2, value, sizeof(ui_meteo_fd2) / sizeof(char));
    ui_meteo_fd2[sizeof(ui_meteo_fd2) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_fd3(void)
{
    return ui_meteo_fd3;
}

void set_var_ui_meteo_fd3(const char *value)
{
    strncpy(ui_meteo_fd3, value, sizeof(ui_meteo_fd3) / sizeof(char));
    ui_meteo_fd3[sizeof(ui_meteo_fd3) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_fd4(void)
{
    return ui_meteo_fd4;
}

void set_var_ui_meteo_fd4(const char *value)
{
    strncpy(ui_meteo_fd4, value, sizeof(ui_meteo_fd4) / sizeof(char));
    ui_meteo_fd4[sizeof(ui_meteo_fd4) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_fd5(void)
{
    return ui_meteo_fd5;
}

void set_var_ui_meteo_fd5(const char *value)
{
    strncpy(ui_meteo_fd5, value, sizeof(ui_meteo_fd5) / sizeof(char));
    ui_meteo_fd5[sizeof(ui_meteo_fd5) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_fd6(void)
{
    return ui_meteo_fd6;
}

void set_var_ui_meteo_fd6(const char *value)
{
    strncpy(ui_meteo_fd6, value, sizeof(ui_meteo_fd6) / sizeof(char));
    ui_meteo_fd6[sizeof(ui_meteo_fd6) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft1_1(void)
{
    return ui_meteo_ft1_1;
}

void set_var_ui_meteo_ft1_1(const char *value)
{
    strncpy(ui_meteo_ft1_1, value, sizeof(ui_meteo_ft1_1) / sizeof(char));
    ui_meteo_ft1_1[sizeof(ui_meteo_ft1_1) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft1_2(void)
{
    return ui_meteo_ft1_2;
}

void set_var_ui_meteo_ft1_2(const char *value)
{
    strncpy(ui_meteo_ft1_2, value, sizeof(ui_meteo_ft1_2) / sizeof(char));
    ui_meteo_ft1_2[sizeof(ui_meteo_ft1_2) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft1_3(void)
{
    return ui_meteo_ft1_3;
}

void set_var_ui_meteo_ft1_3(const char *value)
{
    strncpy(ui_meteo_ft1_3, value, sizeof(ui_meteo_ft1_3) / sizeof(char));
    ui_meteo_ft1_3[sizeof(ui_meteo_ft1_3) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft1_4(void)
{
    return ui_meteo_ft1_4;
}

void set_var_ui_meteo_ft1_4(const char *value)
{
    strncpy(ui_meteo_ft1_4, value, sizeof(ui_meteo_ft1_4) / sizeof(char));
    ui_meteo_ft1_4[sizeof(ui_meteo_ft1_4) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft1_5(void)
{
    return ui_meteo_ft1_5;
}

void set_var_ui_meteo_ft1_5(const char *value)
{
    strncpy(ui_meteo_ft1_5, value, sizeof(ui_meteo_ft1_5) / sizeof(char));
    ui_meteo_ft1_5[sizeof(ui_meteo_ft1_5) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft1_6(void)
{
    return ui_meteo_ft1_6;
}

void set_var_ui_meteo_ft1_6(const char *value)
{
    strncpy(ui_meteo_ft1_6, value, sizeof(ui_meteo_ft1_6) / sizeof(char));
    ui_meteo_ft1_6[sizeof(ui_meteo_ft1_6) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft2_1(void)
{
    return ui_meteo_ft2_1;
}

void set_var_ui_meteo_ft2_1(const char *value)
{
    strncpy(ui_meteo_ft2_1, value, sizeof(ui_meteo_ft2_1) / sizeof(char));
    ui_meteo_ft2_1[sizeof(ui_meteo_ft2_1) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft2_2(void)
{
    return ui_meteo_ft2_2;
}

void set_var_ui_meteo_ft2_2(const char *value)
{
    strncpy(ui_meteo_ft2_2, value, sizeof(ui_meteo_ft2_2) / sizeof(char));
    ui_meteo_ft2_2[sizeof(ui_meteo_ft2_2) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft2_3(void)
{
    return ui_meteo_ft2_3;
}

void set_var_ui_meteo_ft2_3(const char *value)
{
    strncpy(ui_meteo_ft2_3, value, sizeof(ui_meteo_ft2_3) / sizeof(char));
    ui_meteo_ft2_3[sizeof(ui_meteo_ft2_3) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft2_4(void)
{
    return ui_meteo_ft2_4;
}

void set_var_ui_meteo_ft2_4(const char *value)
{
    strncpy(ui_meteo_ft2_4, value, sizeof(ui_meteo_ft2_4) / sizeof(char));
    ui_meteo_ft2_4[sizeof(ui_meteo_ft2_4) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft2_5(void)
{
    return ui_meteo_ft2_5;
}

void set_var_ui_meteo_ft2_5(const char *value)
{
    strncpy(ui_meteo_ft2_5, value, sizeof(ui_meteo_ft2_5) / sizeof(char));
    ui_meteo_ft2_5[sizeof(ui_meteo_ft2_5) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft2_6(void)
{
    return ui_meteo_ft2_6;
}

void set_var_ui_meteo_ft2_6(const char *value)
{
    strncpy(ui_meteo_ft2_6, value, sizeof(ui_meteo_ft2_6) / sizeof(char));
    ui_meteo_ft2_6[sizeof(ui_meteo_ft2_6) / sizeof(char) - 1] = 0;
}

int32_t get_var_ui_start_bar(void)
{
    return ui_start_bar;
}

void set_var_ui_start_bar(int32_t value)
{
    ui_start_bar = value;
}

const char *get_var_ui_start_bar_texte(void)
{
    return ui_start_bar_texte;
}

void set_var_ui_start_bar_texte(const char *value)
{
    strncpy(ui_start_bar_texte, value, sizeof(ui_start_bar_texte) / sizeof(char));
    ui_start_bar_texte[sizeof(ui_start_bar_texte) / sizeof(char) - 1] = 0;
}
