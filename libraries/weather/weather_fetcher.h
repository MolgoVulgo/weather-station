#pragma once

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <time.h>
#include "esp_err.h"

struct CurrentWeatherData {
  float temperature = NAN;
  float feelsLike = NAN;
  float tempMin = NAN;
  float tempMax = NAN;
  float pressure = NAN;
  float pressureSeaLevel = NAN;
  float pressureGroundLevel = NAN;
  uint8_t humidity = 0;
  uint16_t visibility = 0;
  float windKmh = NAN;
  float windDeg = NAN;
  float windGustKmh = NAN;
  uint8_t clouds = 0;
  float rain1h = NAN;
  float rain3h = NAN;
  float snow1h = NAN;
  float snow3h = NAN;
  time_t observationTime = 0;
  time_t sunrise = 0;
  time_t sunset = 0;
  int32_t timezone = 0;
  std::string cityName;
  std::string country;
  std::string main;
  std::string description;
  std::string iconId;
  uint8_t iconVariant = 2;
  int conditionId = 0;
};

struct ForecastEntry {
  time_t timestamp = 0;
  float minTemp = NAN;
  float maxTemp = NAN;
  std::string iconId;
  uint8_t iconVariant = 2;
  int conditionId = 0;
  bool valid = false;
  uint8_t middayOffset = 255;
};

struct MinutelyEntry {
  time_t timestamp = 0;
  float precipitation = NAN;
  bool valid = false;
};

struct HourlyEntry {
  time_t timestamp = 0;
  float temperature = NAN;
  float feelsLike = NAN;
  float pop = NAN;
  uint8_t humidity = 0;
  uint8_t clouds = 0;
  float rain1h = NAN;
  float snow1h = NAN;
  std::string iconId;
  uint8_t iconVariant = 2;
  int conditionId = 0;
  bool valid = false;
};

class WeatherFetcher {
 public:
  WeatherFetcher();
  esp_err_t fetchCurrent(const char* url, CurrentWeatherData& out);
  esp_err_t fetchForecast(const char* url, ForecastEntry* out, size_t count);
  esp_err_t fetchOneCall(const char* url,
                         CurrentWeatherData& current,
                         ForecastEntry* out,
                         size_t count,
                         MinutelyEntry* minutely,
                         size_t minutely_count,
                         HourlyEntry* hourly,
                         size_t hourly_count);
  static bool parseCurrentJson(const char* json, CurrentWeatherData& out);
  static bool parseForecastJson(const char* json, ForecastEntry* out, size_t count);
  static bool parseOneCallJson(const char* json,
                               CurrentWeatherData& current,
                               ForecastEntry* out,
                               size_t count,
                               MinutelyEntry* minutely,
                               size_t minutely_count,
                               HourlyEntry* hourly,
                               size_t hourly_count);
  const std::string& lastError() const { return lastError_; }
  void set_timeout_ms(int timeout_ms) { timeout_ms_ = timeout_ms; }

 private:
  esp_err_t fetchUrl(const char* url, std::string& out);
  std::string lastError_;
  int timeout_ms_ = 8000;
};
