#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;
uint32_t active_theme_index = 0;

static void event_handler_cb_ui_setting_ui_setting_hour(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_ui_setting_hour(value);
        }
    }
}

static void event_handler_cb_ui_setting_ui_setting_gmt_switch(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_slider_get_value(ta);
            set_var_ui_setting_gmt(value);
        }
    }
}

static void event_handler_cb_ui_setting_ui_setting_unit(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_ui_setting_temp(value);
        }
    }
}

static void event_handler_cb_ui_setting_ui_setting_laguage(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_dropdown_get_selected(ta);
            set_var_ui_setting_laguage(value);
        }
    }
}

void create_screen_ui_start() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.ui_start = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 320);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ui_start_bar
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.ui_start_bar = obj;
            lv_obj_set_pos(obj, 165, 155);
            lv_obj_set_size(obj, 150, 10);
        }
        {
            // ui_start_bar_texte
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_start_bar_texte = obj;
            lv_obj_set_pos(obj, 140, 127);
            lv_obj_set_size(obj, 200, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 182, 71);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, _("Station Météo"));
        }
    }
    
    tick_screen_ui_start();
}

void tick_screen_ui_start() {
    {
        int32_t new_val = get_var_ui_start_bar();
        int32_t cur_val = lv_bar_get_value(objects.ui_start_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.ui_start_bar;
            lv_bar_set_value(objects.ui_start_bar, new_val, LV_ANIM_ON);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_start_bar_texte();
        const char *cur_val = lv_label_get_text(objects.ui_start_bar_texte);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_start_bar_texte;
            lv_label_set_text(objects.ui_start_bar_texte, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_ui_meteo() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.ui_meteo = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 320);
    lv_obj_add_event_cb(obj, action_ui_swipe, LV_EVENT_GESTURE, (void *)0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ui_meteo_clock
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_clock = obj;
            lv_obj_set_pos(obj, 235, 15);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_EDITED);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_EDITED);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_img
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.ui_meteo_img = obj;
            lv_obj_set_pos(obj, 31, 16);
            lv_obj_set_size(obj, 150, 150);
            lv_img_set_src(obj, &img_clear_day);
        }
        {
            // ui_meteo_date
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_date = obj;
            lv_obj_set_pos(obj, 235, 70);
            lv_obj_set_size(obj, 200, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_temp
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_temp = obj;
            lv_obj_set_pos(obj, 255, 104);
            lv_obj_set_size(obj, 160, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_condition
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_condition = obj;
            lv_obj_set_pos(obj, 185, 150);
            lv_obj_set_size(obj, 300, LV_SIZE_CONTENT);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_DOT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.obj1 = obj;
            lv_obj_set_pos(obj, 25, 183);
            lv_obj_set_size(obj, 430, LV_SIZE_CONTENT);
            static lv_point_t line_points[] = {
                { 0, 0 },
                { 430, 0 }
            };
            lv_line_set_points(obj, line_points, 2);
            lv_obj_set_style_line_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // ui_meteo_fi1
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.ui_meteo_fi1 = obj;
            lv_obj_set_pos(obj, 27, 205);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_clear_day_50);
        }
        {
            // ui_meteo_fi2
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.ui_meteo_fi2 = obj;
            lv_obj_set_pos(obj, 102, 205);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_clear_day_50);
        }
        {
            // ui_meteo_fi3
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.ui_meteo_fi3 = obj;
            lv_obj_set_pos(obj, 177, 205);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_clear_day_50);
        }
        {
            // ui_meteo_fi4
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.ui_meteo_fi4 = obj;
            lv_obj_set_pos(obj, 252, 205);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_clear_day_50);
        }
        {
            // ui_meteo_fi5
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.ui_meteo_fi5 = obj;
            lv_obj_set_pos(obj, 327, 205);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_clear_day_50);
        }
        {
            // ui_meteo_fi6
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.ui_meteo_fi6 = obj;
            lv_obj_set_pos(obj, 402, 205);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_clear_day_50);
        }
        {
            // ui_meteo_ft1_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_1 = obj;
            lv_obj_set_pos(obj, 16, 256);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.obj2 = obj;
            lv_obj_set_pos(obj, 24, 282);
            lv_obj_set_size(obj, 430, LV_SIZE_CONTENT);
            static lv_point_t line_points[] = {
                { 0, 0 },
                { 430, 0 }
            };
            lv_line_set_points(obj, line_points, 2);
            lv_obj_set_style_line_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // ui_meteo_fd1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_fd1 = obj;
            lv_obj_set_pos(obj, 40, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_fd2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_fd2 = obj;
            lv_obj_set_pos(obj, 115, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_fd3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_fd3 = obj;
            lv_obj_set_pos(obj, 190, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_fd4
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_fd4 = obj;
            lv_obj_set_pos(obj, 265, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_fd5
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_fd5 = obj;
            lv_obj_set_pos(obj, 340, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_fd6
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_fd6 = obj;
            lv_obj_set_pos(obj, 415, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_setting
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.ui_meteo_setting = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 25, 25);
            lv_img_set_src(obj, &img_gear);
            lv_obj_add_event_cb(obj, action_go_setting, LV_EVENT_PRESSED, (void *)0);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        }
        {
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.obj3 = obj;
            lv_obj_set_pos(obj, 88, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            static lv_point_t line_points[] = {
                { 0, 0 },
                { 0, 90 }
            };
            lv_line_set_points(obj, line_points, 2);
            lv_obj_set_style_line_color(obj, lv_color_hex(0xff767676), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 125, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.obj4 = obj;
            lv_obj_set_pos(obj, 163, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            static lv_point_t line_points[] = {
                { 0, 0 },
                { 0, 90 }
            };
            lv_line_set_points(obj, line_points, 2);
            lv_obj_set_style_line_color(obj, lv_color_hex(0xff767676), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 125, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.obj5 = obj;
            lv_obj_set_pos(obj, 239, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            static lv_point_t line_points[] = {
                { 0, 0 },
                { 0, 90 }
            };
            lv_line_set_points(obj, line_points, 2);
            lv_obj_set_style_line_color(obj, lv_color_hex(0xff767676), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 125, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.obj6 = obj;
            lv_obj_set_pos(obj, 313, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            static lv_point_t line_points[] = {
                { 0, 0 },
                { 0, 90 }
            };
            lv_line_set_points(obj, line_points, 2);
            lv_obj_set_style_line_color(obj, lv_color_hex(0xff767676), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 125, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.obj7 = obj;
            lv_obj_set_pos(obj, 388, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            static lv_point_t line_points[] = {
                { 0, 0 },
                { 0, 90 }
            };
            lv_line_set_points(obj, line_points, 2);
            lv_obj_set_style_line_color(obj, lv_color_hex(0xff767676), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 125, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // ui_meteo_ft1_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_2 = obj;
            lv_obj_set_pos(obj, 91, 256);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_ft1_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_3 = obj;
            lv_obj_set_pos(obj, 166, 256);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_ft1_4
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_4 = obj;
            lv_obj_set_pos(obj, 241, 256);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_ft1_5
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_5 = obj;
            lv_obj_set_pos(obj, 316, 256);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_ft1_6
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_6 = obj;
            lv_obj_set_pos(obj, 391, 256);
            lv_obj_set_size(obj, 72, LV_SIZE_CONTENT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.obj8 = obj;
            lv_obj_set_pos(obj, 230, 293);
            lv_obj_set_size(obj, 18, 16);
            lv_led_set_color(obj, lv_color_hex(0xffffffff));
            lv_led_set_brightness(obj, 0);
        }
        {
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.obj9 = obj;
            lv_obj_set_pos(obj, 268, 293);
            lv_obj_set_size(obj, 18, 16);
            lv_led_set_color(obj, lv_color_hex(0xffffffff));
            lv_led_set_brightness(obj, 0);
        }
        {
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.obj10 = obj;
            lv_obj_set_pos(obj, 193, 293);
            lv_obj_set_size(obj, 18, 16);
            lv_led_set_color(obj, lv_color_hex(0xffffffff));
            lv_led_set_brightness(obj, 255);
        }
    }
    
    tick_screen_ui_meteo();
}

void tick_screen_ui_meteo() {
    {
        const char *new_val = get_var_ui_meteo_houre();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_clock);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_clock;
            lv_label_set_text(objects.ui_meteo_clock, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_date();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_date);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_date;
            lv_label_set_text(objects.ui_meteo_date, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_temp();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_temp);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_temp;
            lv_label_set_text(objects.ui_meteo_temp, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_condition();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_condition);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_condition;
            lv_label_set_text(objects.ui_meteo_condition, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft1();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft1_1);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft1_1;
            lv_label_set_text(objects.ui_meteo_ft1_1, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_fd1();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_fd1);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_fd1;
            lv_label_set_text(objects.ui_meteo_fd1, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_fd2();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_fd2);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_fd2;
            lv_label_set_text(objects.ui_meteo_fd2, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_fd3();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_fd3);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_fd3;
            lv_label_set_text(objects.ui_meteo_fd3, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_fd4();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_fd4);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_fd4;
            lv_label_set_text(objects.ui_meteo_fd4, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_fd5();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_fd5);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_fd5;
            lv_label_set_text(objects.ui_meteo_fd5, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_fd6();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_fd6);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_fd6;
            lv_label_set_text(objects.ui_meteo_fd6, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft2();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft1_2);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft1_2;
            lv_label_set_text(objects.ui_meteo_ft1_2, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft3();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft1_3);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft1_3;
            lv_label_set_text(objects.ui_meteo_ft1_3, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft4();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft1_4);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft1_4;
            lv_label_set_text(objects.ui_meteo_ft1_4, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft5();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft1_5);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft1_5;
            lv_label_set_text(objects.ui_meteo_ft1_5, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft5();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft1_6);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft1_6;
            lv_label_set_text(objects.ui_meteo_ft1_6, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_ui_meteo_details() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.ui_meteo_details = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 320);
    lv_obj_add_event_cb(obj, action_ui_swipe, LV_EVENT_GESTURE, (void *)0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 190, 24);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "ui meteo 2");
        }
        {
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.obj11 = obj;
            lv_obj_set_pos(obj, 24, 282);
            lv_obj_set_size(obj, 430, LV_SIZE_CONTENT);
            static lv_point_t line_points[] = {
                { 0, 0 },
                { 430, 0 }
            };
            lv_line_set_points(obj, line_points, 2);
            lv_obj_set_style_line_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.obj12 = obj;
            lv_obj_set_pos(obj, 230, 293);
            lv_obj_set_size(obj, 18, 16);
            lv_led_set_color(obj, lv_color_hex(0xffffffff));
            lv_led_set_brightness(obj, 255);
        }
        {
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.obj13 = obj;
            lv_obj_set_pos(obj, 268, 293);
            lv_obj_set_size(obj, 18, 16);
            lv_led_set_color(obj, lv_color_hex(0xffffffff));
            lv_led_set_brightness(obj, 0);
        }
        {
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.obj14 = obj;
            lv_obj_set_pos(obj, 193, 293);
            lv_obj_set_size(obj, 18, 16);
            lv_led_set_color(obj, lv_color_hex(0xffffffff));
            lv_led_set_brightness(obj, 0);
        }
    }
    
    tick_screen_ui_meteo_details();
}

void tick_screen_ui_meteo_details() {
}

void create_screen_ui_setting() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.ui_setting = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 320);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(obj, &ui_font_ui_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj15 = obj;
            lv_obj_set_pos(obj, 218, 73);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "12");
        }
        {
            // ui_setting_hour
            lv_obj_t *obj = lv_switch_create(parent_obj);
            objects.ui_setting_hour = obj;
            lv_obj_set_pos(obj, 241, 72);
            lv_obj_set_size(obj, 39, 20);
            lv_obj_add_event_cb(obj, action_checked, LV_EVENT_VALUE_CHANGED, (void *)0);
            lv_obj_add_event_cb(obj, event_handler_cb_ui_setting_ui_setting_hour, LV_EVENT_ALL, 0);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff007cff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff007cff), LV_PART_MAIN | LV_STATE_CHECKED);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj16 = obj;
            lv_obj_set_pos(obj, 280, 73);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "24");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 42, 73);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, _("Format de l'heure :"));
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 42, 97);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, _("Décalage horaire :"));
        }
        {
            // ui_setting_gmt_switch
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.ui_setting_gmt_switch = obj;
            lv_obj_set_pos(obj, 108, 148);
            lv_obj_set_size(obj, 263, 10);
            lv_slider_set_range(obj, -12, 14);
            lv_slider_set_mode(obj, LV_SLIDER_MODE_SYMMETRICAL);
            lv_obj_add_event_cb(obj, event_handler_cb_ui_setting_ui_setting_gmt_switch, LV_EVENT_ALL, 0);
        }
        {
            // ui_setting_gmt_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_setting_gmt_label = obj;
            lv_obj_set_pos(obj, 218, 97);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 94, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "-12");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 225, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "0");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 357, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "+12");
        }
        {
            // ui_setting_gmt_switch1
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.ui_setting_gmt_switch1 = obj;
            lv_obj_set_pos(obj, 65, 141);
            lv_obj_set_size(obj, 29, 25);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, -1, 3);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "-");
                }
            }
        }
        {
            // ui_setting_gmt_switch2
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.ui_setting_gmt_switch2 = obj;
            lv_obj_set_pos(obj, 387, 141);
            lv_obj_set_size(obj, 29, 25);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 3);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "+");
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 42, 179);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_pad_right(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, _("Unité :"));
        }
        {
            // ui_setting_unit
            lv_obj_t *obj = lv_switch_create(parent_obj);
            objects.ui_setting_unit = obj;
            lv_obj_set_pos(obj, 137, 178);
            lv_obj_set_size(obj, 39, 20);
            lv_obj_add_event_cb(obj, action_check_unit, LV_EVENT_VALUE_CHANGED, (void *)0);
            lv_obj_add_event_cb(obj, event_handler_cb_ui_setting_ui_setting_unit, LV_EVENT_ALL, 0);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff007cff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff007cff), LV_PART_MAIN | LV_STATE_CHECKED);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 176, 179);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_pad_left(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "°F");
        }
        {
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.obj17 = obj;
            lv_obj_set_pos(obj, 24, 282);
            lv_obj_set_size(obj, 430, LV_SIZE_CONTENT);
            static lv_point_t line_points[] = {
                { 0, 0 },
                { 430, 0 }
            };
            lv_line_set_points(obj, line_points, 2);
            lv_obj_set_style_line_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.obj18 = obj;
            lv_obj_set_pos(obj, 230, 293);
            lv_obj_set_size(obj, 18, 16);
            lv_led_set_color(obj, lv_color_hex(0xffffffff));
            lv_led_set_brightness(obj, 0);
        }
        {
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.obj19 = obj;
            lv_obj_set_pos(obj, 268, 293);
            lv_obj_set_size(obj, 18, 16);
            lv_led_set_color(obj, lv_color_hex(0xffffffff));
            lv_led_set_brightness(obj, 255);
        }
        {
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.obj20 = obj;
            lv_obj_set_pos(obj, 193, 293);
            lv_obj_set_size(obj, 18, 16);
            lv_led_set_color(obj, lv_color_hex(0xffffffff));
            lv_led_set_brightness(obj, 0);
        }
        {
            // ui_setting_laguage
            lv_obj_t *obj = lv_dropdown_create(parent_obj);
            objects.ui_setting_laguage = obj;
            lv_obj_set_pos(obj, 48, 220);
            lv_obj_set_size(obj, 150, 35);
            lv_dropdown_set_options(obj, "Francais\nanglais\nAllemand");
            lv_obj_add_event_cb(obj, event_handler_cb_ui_setting_ui_setting_laguage, LV_EVENT_ALL, 0);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 112, 179);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "°C");
        }
        {
            // ui_settings_save
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.ui_settings_save = obj;
            lv_obj_set_pos(obj, 348, 220);
            lv_obj_set_size(obj, 100, 50);
            lv_obj_add_event_cb(obj, action_go_ui_meteo, LV_EVENT_PRESSED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Save");
                }
            }
        }
    }
    
    tick_screen_ui_setting();
}

void tick_screen_ui_setting() {
    {
        bool new_val = get_var_ui_setting_hour();
        bool cur_val = lv_obj_has_state(objects.ui_setting_hour, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.ui_setting_hour;
            if (new_val) lv_obj_add_state(objects.ui_setting_hour, LV_STATE_CHECKED);
            else lv_obj_clear_state(objects.ui_setting_hour, LV_STATE_CHECKED);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_ui_setting_gmt();
        int32_t cur_val = lv_slider_get_value(objects.ui_setting_gmt_switch);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.ui_setting_gmt_switch;
            lv_slider_set_value(objects.ui_setting_gmt_switch, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_setting_gmt_txt();
        const char *cur_val = lv_label_get_text(objects.ui_setting_gmt_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_setting_gmt_label;
            lv_label_set_text(objects.ui_setting_gmt_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_ui_setting_temp();
        bool cur_val = lv_obj_has_state(objects.ui_setting_unit, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.ui_setting_unit;
            if (new_val) lv_obj_add_state(objects.ui_setting_unit, LV_STATE_CHECKED);
            else lv_obj_clear_state(objects.ui_setting_unit, LV_STATE_CHECKED);
            tick_value_change_obj = NULL;
        }
    }
    {
        if (!(lv_obj_get_state(objects.ui_setting_laguage) & LV_STATE_EDITED)) {
            int32_t new_val = get_var_ui_setting_laguage();
            int32_t cur_val = lv_dropdown_get_selected(objects.ui_setting_laguage);
            if (new_val != cur_val) {
                tick_value_change_obj = objects.ui_setting_laguage;
                lv_dropdown_set_selected(objects.ui_setting_laguage, new_val);
                tick_value_change_obj = NULL;
            }
        }
    }
}

void create_screen_ui_wifi() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.ui_wifi = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 320);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_qrcode_create(parent_obj, 160, lv_color_hex(0xff000000), lv_color_hex(0xffe2f5fe));
            lv_obj_set_pos(obj, 160, 80);
            lv_obj_set_size(obj, 160, 160);
            lv_qrcode_update(obj, "WIFI:T:WPA;S:StationMeteo;P:stationmeteo;;", 42);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_line_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj21 = obj;
            lv_obj_set_pos(obj, 261, 240);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Avancer");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj22 = obj;
            lv_obj_set_pos(obj, 159, 31);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Configuration Wifi");
        }
    }
    
    tick_screen_ui_wifi();
}

void tick_screen_ui_wifi() {
}



typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_ui_start,
    tick_screen_ui_meteo,
    tick_screen_ui_meteo_details,
    tick_screen_ui_setting,
    tick_screen_ui_wifi,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_ui_start();
    create_screen_ui_meteo();
    create_screen_ui_meteo_details();
    create_screen_ui_setting();
    create_screen_ui_wifi();
}
