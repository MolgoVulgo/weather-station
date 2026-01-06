#include "weather_icons.h"

#include <cstring>

const WeatherIconAsset* FindWeatherIconAsset(const String& iconId) {
  return FindWeatherIconAsset(iconId.c_str());
}

const WeatherIconAsset* FindWeatherIconAsset(const char* iconId) {
  if (iconId == nullptr || *iconId == '\0') {
    return nullptr;
  }

  for (size_t i = 0; i < kWeatherIconAssetsCount; ++i) {
    if (strcmp(kWeatherIconAssets[i].id, iconId) == 0) {
      return &kWeatherIconAssets[i];
    }
  }
  return nullptr;
}
