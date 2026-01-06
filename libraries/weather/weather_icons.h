#pragma once

#include <Arduino.h>

struct WeatherIconAsset {
  const char* id;
  uint16_t width;
  uint16_t height;
  const char* filePath;  // Chemin dans LittleFS
};

extern const WeatherIconAsset kWeatherIconAssets[];
extern const size_t kWeatherIconAssetsCount;

const WeatherIconAsset* FindWeatherIconAsset(const String& iconId);
const WeatherIconAsset* FindWeatherIconAsset(const char* iconId);
