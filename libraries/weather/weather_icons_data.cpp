#include "weather_icons.h"

#define ICON_SET_PATH "/icons/"
const WeatherIconAsset kWeatherIconAssets[] = {
    {"cloudy", 100, 100, ICON_SET_PATH "cloudy.bin"},
    {"day_clear", 100, 100, ICON_SET_PATH "day_clear.bin"},
    {"day_partial_cloud", 100, 100, ICON_SET_PATH "day_partial_cloud.bin"},
    {"day_rain", 100, 100, ICON_SET_PATH "day_rain.bin"},
    {"day_rain_thunder", 100, 100, ICON_SET_PATH "day_rain_thunder.bin"},
    {"day_snow", 100, 100, ICON_SET_PATH "day_snow.bin"},
    {"day_snow_thunder", 100, 100, ICON_SET_PATH "day_snow_thunder.bin"},
    {"mist", 100, 100, ICON_SET_PATH "mist.bin"},
    {"night_clear", 100, 100, ICON_SET_PATH "night_clear.bin"},
    {"night_partial_cloud", 100, 100, ICON_SET_PATH "night_partial_cloud.bin"},
    {"night_rain", 100, 100, ICON_SET_PATH "night_rain.bin"},
    {"night_rain_thunder", 100, 100, ICON_SET_PATH "night_rain_thunder.bin"},
    {"night_snow", 100, 100, ICON_SET_PATH "night_snow.bin"},
    {"night_snow_thunder", 100, 100, ICON_SET_PATH "night_snow_thunder.bin"},
    {"overcast", 100, 100, ICON_SET_PATH "overcast.bin"},
    {"rain", 100, 100, ICON_SET_PATH "rain.bin"},
    {"snow", 100, 100, ICON_SET_PATH "snow.bin"},
};

const size_t kWeatherIconAssetsCount = sizeof(kWeatherIconAssets) / sizeof(kWeatherIconAssets[0]);
