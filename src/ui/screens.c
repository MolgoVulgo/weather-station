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
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, "s:/ui_image_clear_day.bin");
        }
        {
            // ui_meteo_date
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_date = obj;
            lv_obj_set_pos(obj, 235, 70);
            lv_obj_set_size(obj, 200, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ui_meteo_temp
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_temp = obj;
            lv_obj_set_pos(obj, 255, 107);
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
            lv_obj_set_pos(obj, 210, 150);
            lv_obj_set_size(obj, 250, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
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
            // ui_meteo_fi_1
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.ui_meteo_fi_1 = obj;
            lv_obj_set_pos(obj, 27, 205);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, "s:/ui_image_clear_day_50.bin");
        }
        {
            // ui_meteo_fi_2
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.ui_meteo_fi_2 = obj;
            lv_obj_set_pos(obj, 102, 205);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, "s:/ui_image_clear_day_50.bin");
        }
        {
            // ui_meteo_fi_3
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.ui_meteo_fi_3 = obj;
            lv_obj_set_pos(obj, 177, 205);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, "s:/ui_image_clear_day_50.bin");
        }
        {
            // ui_meteo_fi_4
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.ui_meteo_fi_4 = obj;
            lv_obj_set_pos(obj, 252, 205);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, "s:/ui_image_clear_day_50.bin");
        }
        {
            // ui_meteo_fi_5
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.ui_meteo_fi_5 = obj;
            lv_obj_set_pos(obj, 327, 205);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, "s:/ui_image_clear_day_50.bin");
        }
        {
            // ui_meteo_fi_6
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.ui_meteo_fi_6 = obj;
            lv_obj_set_pos(obj, 402, 205);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, "s:/ui_image_clear_day_50.bin");
        }
        {
            // ui_meteo_ft1_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_1 = obj;
            lv_obj_set_pos(obj, 34, 255);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-99.1°");
        }
        {
            // ui_meteo_ft2_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft2_1 = obj;
            lv_obj_set_pos(obj, 34, 270);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-99.1°");
        }
        {
            // ui_meteo_ft1_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_2 = obj;
            lv_obj_set_pos(obj, 109, 255);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-99.1°");
        }
        {
            // ui_meteo_ft2_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft2_2 = obj;
            lv_obj_set_pos(obj, 109, 270);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-99.1°");
        }
        {
            // ui_meteo_ft1_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_3 = obj;
            lv_obj_set_pos(obj, 184, 255);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-99.1°");
        }
        {
            // ui_meteo_ft2_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft2_3 = obj;
            lv_obj_set_pos(obj, 184, 270);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-99.1°");
        }
        {
            // ui_meteo_ft1_4
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_4 = obj;
            lv_obj_set_pos(obj, 259, 255);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-99.1°");
        }
        {
            // ui_meteo_ft2_4
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft2_4 = obj;
            lv_obj_set_pos(obj, 259, 270);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-99.1°");
        }
        {
            // ui_meteo_ft1_5
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_5 = obj;
            lv_obj_set_pos(obj, 334, 255);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-99.1°");
        }
        {
            // ui_meteo_ft2_5
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft2_5 = obj;
            lv_obj_set_pos(obj, 334, 270);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-99.1°");
        }
        {
            // ui_meteo_ft1_6
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft1_6 = obj;
            lv_obj_set_pos(obj, 409, 255);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-99.1°");
        }
        {
            // ui_meteo_ft2_6
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ui_meteo_ft2_6 = obj;
            lv_obj_set_pos(obj, 409, 270);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-99.1°");
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
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj2 = obj;
            lv_obj_set_pos(obj, 37, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "LUN");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj3 = obj;
            lv_obj_set_pos(obj, 112, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "LUN");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj4 = obj;
            lv_obj_set_pos(obj, 187, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "LUN");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj5 = obj;
            lv_obj_set_pos(obj, 262, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "LUN");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj6 = obj;
            lv_obj_set_pos(obj, 337, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "LUN");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj7 = obj;
            lv_obj_set_pos(obj, 412, 190);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "LUN");
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
