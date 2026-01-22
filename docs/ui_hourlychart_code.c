// Extracted from runtime chart setup (LVGL 8.3.11)
// Purpose: create/configure the ui_hourlychart chart only.

#include "lvgl.h"

// Caller owns the screen/parent; returns created chart object.
lv_obj_t * ui_hourlychart_create(lv_obj_t * parent)
{
    lv_obj_t * ui_hourlychart = lv_chart_create(parent);
    lv_coord_t parent_w = lv_obj_get_width(parent);
    if (parent_w > 400) {
        parent_w = 400;
    }
    lv_obj_set_width(ui_hourlychart, parent_w);
    lv_obj_set_height(ui_hourlychart, 100);
    lv_obj_set_pos(ui_hourlychart, 10, 0);
    lv_obj_set_align(ui_hourlychart, LV_ALIGN_CENTER);
    lv_chart_set_type(ui_hourlychart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(ui_hourlychart, 7);
    lv_chart_set_range(ui_hourlychart, LV_CHART_AXIS_PRIMARY_Y, 0, 20);
    lv_chart_set_range(ui_hourlychart, LV_CHART_AXIS_SECONDARY_Y, 0, 0);
    lv_chart_set_div_line_count(ui_hourlychart, 5, 7);
    lv_chart_set_axis_tick(ui_hourlychart, LV_CHART_AXIS_PRIMARY_X, 10, 2, 7, 3, false, 50);
    lv_chart_set_axis_tick(ui_hourlychart, LV_CHART_AXIS_PRIMARY_Y, 10, 5, 5, 2, true, 50);
    lv_chart_set_axis_tick(ui_hourlychart, LV_CHART_AXIS_SECONDARY_Y, 0, 0, 0, 0, false, 25);
    lv_obj_set_style_bg_color(ui_hourlychart, lv_color_hex(0x141b1e), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_hourlychart, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_hourlychart, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(ui_hourlychart, lv_color_hex(0x2a3336), LV_PART_MAIN);
    lv_obj_set_style_line_color(ui_hourlychart, lv_color_hex(0x263034), LV_PART_MAIN);
    lv_obj_set_style_text_color(ui_hourlychart, lv_color_hex(0xd0d0d0), LV_PART_TICKS);

    lv_chart_series_t * ui_hourlychart_series_1 = lv_chart_add_series(
        ui_hourlychart,
        lv_color_hex(0xb7b7b7),
        LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_series_color(ui_hourlychart, ui_hourlychart_series_1, lv_color_hex(0xb7b7b7));
    lv_obj_set_style_bg_color(ui_hourlychart, lv_color_hex(0xc2c2c2), LV_PART_INDICATOR);
    lv_obj_set_style_border_color(ui_hourlychart, lv_color_hex(0x9fa4a6), LV_PART_INDICATOR);
    lv_obj_set_style_border_width(ui_hourlychart, 1, LV_PART_INDICATOR);
    lv_obj_set_style_size(ui_hourlychart, 6, LV_PART_INDICATOR);

    static lv_coord_t ui_hourlychart_series_1_array[] = { 2, 5, 8, 9, 7, 6, 3 };
    lv_chart_set_ext_y_array(ui_hourlychart, ui_hourlychart_series_1, ui_hourlychart_series_1_array);

    return ui_hourlychart;
}
