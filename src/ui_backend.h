#ifndef UI_BACKEND_H
#define UI_BACKEND_H

#include <lvgl.h>

#include "ui/ui.h"
#include "ui/screens.h"

static inline lv_obj_t *ui_time_label(void)
{
    return objects.ui_meteo_clock;
}

static inline lv_obj_t *ui_weather_image(void)
{
    return objects.ui_meteo_img;
}

static inline lv_obj_t *ui_weather_forecast_icon(size_t index)
{
    switch (index) {
    case 0:
        return objects.ui_meteo_fi1;
    case 1:
        return objects.ui_meteo_fi2;
    case 2:
        return objects.ui_meteo_fi3;
    case 3:
        return objects.ui_meteo_fi4;
    case 4:
        return objects.ui_meteo_fi5;
    case 5:
        return objects.ui_meteo_fi6;
    default:
        return NULL;
    }
}

#endif
