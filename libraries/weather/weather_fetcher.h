#pragma once

#include <Arduino.h>
#include <WiFiClientSecureBearSSL.h>
#include <time.h>

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
  String cityName;
  String country;
  String main;
  String description;
  String iconId;
  int conditionId = 0;
};

struct ForecastEntry {
  time_t timestamp = 0;
  float minTemp = NAN;
  float maxTemp = NAN;
  String iconId;
  int conditionId = 0;
  bool valid = false;
  uint8_t middayOffset = 255;
};

class WeatherFetcher {
 public:
  explicit WeatherFetcher(BearSSL::WiFiClientSecure& client);
  bool fetchCurrent(const String& url, CurrentWeatherData& out);
  bool fetchForecast(const String& url, ForecastEntry* out, size_t count);
  const String& lastError() const { return lastError_; }

 private:
  BearSSL::WiFiClientSecure& client_;
  String lastError_;
};
