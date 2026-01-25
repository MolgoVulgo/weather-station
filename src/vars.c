#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "temp_unit.h"
#include "time_sync.h"
#include "vars.h"
#include "lanague.h"
#include "i18n.h"
#include "esp_system.h"
#include "ui_settings.h"

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
char ui_meteo_ft1[32] = { 0 };
char ui_meteo_ft2[32] = { 0 };
char ui_meteo_ft3[32] = { 0 };
char ui_meteo_ft4[32] = { 0 };
char ui_meteo_ft5[32] = { 0 };
char ui_meteo_ft6[32] = { 0 };
char ui_humidity[16] = { 0 };
char ui_clouds[16] = { 0 };
char ui_pop[16] = { 0 };
char ui_start_bar_texte[64] = { 0 };
int32_t ui_start_bar = 0;
int32_t ui_setting_gmt = 0;
char ui_setting_gmt_txt[16] = { 0 };
bool ui_setting_hour = true;
bool ui_setting_temp = false;
int32_t ui_setting_laguage = LANAGUE_FR;
static bool s_lang_restart_enabled = false;

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

const char *get_var_ui_meteo_ft1(void)
{
    return ui_meteo_ft1;
}

void set_var_ui_meteo_ft1(const char *value)
{
    strncpy(ui_meteo_ft1, value, sizeof(ui_meteo_ft1) / sizeof(char));
    ui_meteo_ft1[sizeof(ui_meteo_ft1) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft2(void)
{
    return ui_meteo_ft2;
}

void set_var_ui_meteo_ft2(const char *value)
{
    strncpy(ui_meteo_ft2, value, sizeof(ui_meteo_ft2) / sizeof(char));
    ui_meteo_ft2[sizeof(ui_meteo_ft2) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft3(void)
{
    return ui_meteo_ft3;
}

void set_var_ui_meteo_ft3(const char *value)
{
    strncpy(ui_meteo_ft3, value, sizeof(ui_meteo_ft3) / sizeof(char));
    ui_meteo_ft3[sizeof(ui_meteo_ft3) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft4(void)
{
    return ui_meteo_ft4;
}

void set_var_ui_meteo_ft4(const char *value)
{
    strncpy(ui_meteo_ft4, value, sizeof(ui_meteo_ft4) / sizeof(char));
    ui_meteo_ft4[sizeof(ui_meteo_ft4) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft5(void)
{
    return ui_meteo_ft5;
}

void set_var_ui_meteo_ft5(const char *value)
{
    strncpy(ui_meteo_ft5, value, sizeof(ui_meteo_ft5) / sizeof(char));
    ui_meteo_ft5[sizeof(ui_meteo_ft5) / sizeof(char) - 1] = 0;
}

const char *get_var_ui_meteo_ft6(void)
{
    return ui_meteo_ft6;
}

void set_var_ui_meteo_ft6(const char *value)
{
    strncpy(ui_meteo_ft6, value, sizeof(ui_meteo_ft6) / sizeof(char));
    ui_meteo_ft6[sizeof(ui_meteo_ft6) / sizeof(char) - 1] = 0;
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

int32_t get_var_ui_setting_gmt(void)
{
    return ui_setting_gmt;
}

void set_var_ui_setting_gmt(int32_t value)
{
    bool changed = (ui_setting_gmt != value);
    ui_setting_gmt = value;
    snprintf(ui_setting_gmt_txt, sizeof(ui_setting_gmt_txt), "%+" PRId32, value);
    ui_setting_gmt_txt[sizeof(ui_setting_gmt_txt) - 1] = 0;
    if (changed) {
        time_sync_set_tz_offset_seconds(value * 3600);
        time_sync_save_config();
    }
}

bool get_var_ui_setting_hour(void)
{
    return ui_setting_hour;
}

void set_var_ui_setting_hour(bool value)
{
    bool changed = (ui_setting_hour != value);
    ui_setting_hour = value;
    if (changed) {
        time_sync_set_hour_format_24h(value);
        time_sync_save_config();
    }
}

bool get_var_ui_setting_temp(void)
{
    return ui_setting_temp;
}

void set_var_ui_setting_temp(bool value)
{
    bool changed = (ui_setting_temp != value);
    ui_setting_temp = value;
    if (changed) {
        temp_unit_set_fahrenheit(value);
        if (temp_unit_has_last()) {
            char temp_buf[16];
            temp_unit_format(temp_unit_get_last_c(), temp_buf, sizeof(temp_buf));
            if (temp_buf[0] != '\0') {
                set_var_ui_meteo_temp(temp_buf);
            }
        }
    }
}

const char *get_var_ui_setting_gmt_txt(void)
{
    return ui_setting_gmt_txt;
}

void set_var_ui_setting_gmt_txt(const char *value)
{
    strncpy(ui_setting_gmt_txt, value, sizeof(ui_setting_gmt_txt) / sizeof(char));
    ui_setting_gmt_txt[sizeof(ui_setting_gmt_txt) / sizeof(char) - 1] = 0;
}

int32_t get_var_ui_setting_laguage(void)
{
    return ui_setting_laguage;
}

void set_var_ui_setting_laguage(int32_t value)
{
    lanague_id_t lang = LANAGUE_FR;
    if (value == LANAGUE_EN) {
        lang = LANAGUE_EN;
    } else if (value == LANAGUE_DE) {
        lang = LANAGUE_DE;
    }

    bool changed = (ui_setting_laguage != (int32_t)lang);
    ui_setting_laguage = (int32_t)lang;
    if (changed) {
        lanague_set_current(lang);
        lanague_save_current();
        if (s_lang_restart_enabled) {
            esp_restart();
        }
    }
}

void ui_settings_enable_language_restart(bool enable)
{
    s_lang_restart_enabled = enable;
}

int32_t get_var_ui_humidity(void)
{
    return (int32_t)(intptr_t)ui_humidity;
}

void set_var_ui_humidity(int32_t value)
{
    const char *text = (const char *)(intptr_t)value;
    if (!text) {
        text = "";
    }
    strncpy(ui_humidity, text, sizeof(ui_humidity) / sizeof(char));
    ui_humidity[sizeof(ui_humidity) / sizeof(char) - 1] = 0;
}

int32_t get_var_ui_clouds(void)
{
    return (int32_t)(intptr_t)ui_clouds;
}

void set_var_ui_clouds(int32_t value)
{
    const char *text = (const char *)(intptr_t)value;
    if (!text) {
        text = "";
    }
    strncpy(ui_clouds, text, sizeof(ui_clouds) / sizeof(char));
    ui_clouds[sizeof(ui_clouds) / sizeof(char) - 1] = 0;
}

int32_t get_var_ui_pop(void)
{
    return (int32_t)(intptr_t)ui_pop;
}

void set_var_ui_pop(int32_t value)
{
    const char *text = (const char *)(intptr_t)value;
    if (!text) {
        text = "";
    }
    strncpy(ui_pop, text, sizeof(ui_pop) / sizeof(char));
    ui_pop[sizeof(ui_pop) / sizeof(char) - 1] = 0;
}
