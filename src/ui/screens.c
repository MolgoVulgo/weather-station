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

void create_screen_ui_meteo() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.ui_meteo = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 320);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ui_meteo_clock
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_clock = obj;
            lv_obj_set_pos(obj, 235, 15);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
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
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
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
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
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
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.obj0 = obj;
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
            lv_obj_set_pos(obj, 28, 255);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_ft2_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft2_1 = obj;
            lv_obj_set_pos(obj, 28, 270);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_ft1_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_2 = obj;
            lv_obj_set_pos(obj, 103, 255);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_ft2_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft2_2 = obj;
            lv_obj_set_pos(obj, 103, 270);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_ft1_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_3 = obj;
            lv_obj_set_pos(obj, 178, 255);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_ft2_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft2_3 = obj;
            lv_obj_set_pos(obj, 178, 270);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_ft1_4
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_4 = obj;
            lv_obj_set_pos(obj, 253, 255);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_ft2_4
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft2_4 = obj;
            lv_obj_set_pos(obj, 253, 270);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_ft1_5
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_5 = obj;
            lv_obj_set_pos(obj, 328, 255);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_ft2_5
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft2_5 = obj;
            lv_obj_set_pos(obj, 328, 270);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_ft1_6
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_6 = obj;
            lv_obj_set_pos(obj, 403, 255);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_ft2_6
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft2_6 = obj;
            lv_obj_set_pos(obj, 403, 270);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.obj1 = obj;
            lv_obj_set_pos(obj, 24, 292);
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
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_fd2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_fd2 = obj;
            lv_obj_set_pos(obj, 115, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_fd3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_fd3 = obj;
            lv_obj_set_pos(obj, 190, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_fd4
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_fd4 = obj;
            lv_obj_set_pos(obj, 265, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_fd5
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_fd5 = obj;
            lv_obj_set_pos(obj, 340, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_fd6
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_fd6 = obj;
            lv_obj_set_pos(obj, 415, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_ui_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
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
        const char *new_val = get_var_ui_meteo_ft1_1();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft1_1);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft1_1;
            lv_label_set_text(objects.ui_meteo_ft1_1, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft2_1();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft2_1);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft2_1;
            lv_label_set_text(objects.ui_meteo_ft2_1, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft1_2();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft1_2);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft1_2;
            lv_label_set_text(objects.ui_meteo_ft1_2, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft2_2();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft2_2);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft2_2;
            lv_label_set_text(objects.ui_meteo_ft2_2, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft1_3();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft1_3);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft1_3;
            lv_label_set_text(objects.ui_meteo_ft1_3, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft2_3();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft2_3);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft2_3;
            lv_label_set_text(objects.ui_meteo_ft2_3, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft1_4();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft1_4);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft1_4;
            lv_label_set_text(objects.ui_meteo_ft1_4, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft2_4();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft2_4);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft2_4;
            lv_label_set_text(objects.ui_meteo_ft2_4, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft1_5();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft1_5);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft1_5;
            lv_label_set_text(objects.ui_meteo_ft1_5, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft2_5();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft2_5);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft2_5;
            lv_label_set_text(objects.ui_meteo_ft2_5, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft1_6();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft1_6);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft1_6;
            lv_label_set_text(objects.ui_meteo_ft1_6, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ui_meteo_ft2_6();
        const char *cur_val = lv_label_get_text(objects.ui_meteo_ft2_6);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ui_meteo_ft2_6;
            lv_label_set_text(objects.ui_meteo_ft2_6, new_val);
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
}



typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_ui_meteo,
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
    
    create_screen_ui_meteo();
}
