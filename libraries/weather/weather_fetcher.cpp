#include "weather_fetcher.h"

#include <cmath>
#include <cstring>
#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"

namespace {
constexpr float MS_TO_KMH = 3.6f;

static float json_number_or(cJSON *obj, const char *key, float fallback);
static int json_int_or(cJSON *obj, const char *key, int fallback);
static const char* json_string_or(cJSON *obj, const char *key);

static uint8_t map_icon_variant(const char *icon_code) {
  if (icon_code && strlen(icon_code) >= 3) {
    char suffix = icon_code[strlen(icon_code) - 1];
    if (suffix == 'd') {
      return 0;
    }
    if (suffix == 'n') {
      return 1;
    }
  }
  return 2;
}

static void parse_weather_item(cJSON *weather,
                               int *condition_id,
                               std::string *icon_id,
                               uint8_t *variant,
                               std::string *main_text,
                               std::string *description)
{
  if (!cJSON_IsArray(weather) || cJSON_GetArraySize(weather) == 0) {
    return;
  }
  cJSON *item = cJSON_GetArrayItem(weather, 0);
  if (!cJSON_IsObject(item)) {
    return;
  }
  if (condition_id) {
    *condition_id = json_int_or(item, "id", *condition_id);
  }
  const char *icon_code = json_string_or(item, "icon");
  if (icon_code && icon_id) {
    *icon_id = icon_code;
  }
  if (variant) {
    *variant = map_icon_variant(icon_code);
  }
  const char *main_val = json_string_or(item, "main");
  if (main_val && main_text) {
    *main_text = main_val;
  }
  const char *desc_val = json_string_or(item, "description");
  if (desc_val && description) {
    *description = desc_val;
  }
}

static float json_number_or(cJSON *obj, const char *key, float fallback) {
  cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, key);
  if (cJSON_IsNumber(item)) {
    return static_cast<float>(item->valuedouble);
  }
  return fallback;
}

static int json_int_or(cJSON *obj, const char *key, int fallback) {
  cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, key);
  if (cJSON_IsNumber(item)) {
    return item->valueint;
  }
  return fallback;
}

static const char* json_string_or(cJSON *obj, const char *key) {
  cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, key);
  if (cJSON_IsString(item) && item->valuestring) {
    return item->valuestring;
  }
  return nullptr;
}

static bool parse_current(const char *json, CurrentWeatherData &out) {
  cJSON *root = cJSON_Parse(json);
  if (!root) {
    return false;
  }

  cJSON *main = cJSON_GetObjectItemCaseSensitive(root, "main");
  if (cJSON_IsObject(main)) {
    out.temperature = json_number_or(main, "temp", out.temperature);
    out.feelsLike = json_number_or(main, "feels_like", out.feelsLike);
    out.tempMin = json_number_or(main, "temp_min", out.tempMin);
    out.tempMax = json_number_or(main, "temp_max", out.tempMax);
    out.pressure = json_number_or(main, "pressure", out.pressure);
    out.pressureSeaLevel = json_number_or(main, "sea_level", out.pressureSeaLevel);
    out.pressureGroundLevel = json_number_or(main, "grnd_level", out.pressureGroundLevel);
    out.humidity = static_cast<uint8_t>(json_int_or(main, "humidity", out.humidity));
  }

  cJSON *wind = cJSON_GetObjectItemCaseSensitive(root, "wind");
  if (cJSON_IsObject(wind)) {
    out.windKmh = json_number_or(wind, "speed", out.windKmh) * MS_TO_KMH;
    out.windDeg = json_number_or(wind, "deg", out.windDeg);
    out.windGustKmh = json_number_or(wind, "gust", out.windGustKmh) * MS_TO_KMH;
  }

  cJSON *clouds = cJSON_GetObjectItemCaseSensitive(root, "clouds");
  if (cJSON_IsObject(clouds)) {
    out.clouds = static_cast<uint8_t>(json_int_or(clouds, "all", out.clouds));
  }

  cJSON *rain = cJSON_GetObjectItemCaseSensitive(root, "rain");
  if (cJSON_IsObject(rain)) {
    out.rain1h = json_number_or(rain, "1h", out.rain1h);
    out.rain3h = json_number_or(rain, "3h", out.rain3h);
  }

  cJSON *snow = cJSON_GetObjectItemCaseSensitive(root, "snow");
  if (cJSON_IsObject(snow)) {
    out.snow1h = json_number_or(snow, "1h", out.snow1h);
    out.snow3h = json_number_or(snow, "3h", out.snow3h);
  }

  cJSON *sys = cJSON_GetObjectItemCaseSensitive(root, "sys");
  if (cJSON_IsObject(sys)) {
    out.sunrise = static_cast<time_t>(json_int_or(sys, "sunrise", out.sunrise));
    out.sunset = static_cast<time_t>(json_int_or(sys, "sunset", out.sunset));
    const char *country = json_string_or(sys, "country");
    if (country) {
      out.country = country;
    }
  }

  const char *icon_code = nullptr;
  cJSON *weather = cJSON_GetObjectItemCaseSensitive(root, "weather");
  if (cJSON_IsArray(weather) && cJSON_GetArraySize(weather) > 0) {
    cJSON *item = cJSON_GetArrayItem(weather, 0);
    if (cJSON_IsObject(item)) {
      out.conditionId = json_int_or(item, "id", out.conditionId);
      const char *main_txt = json_string_or(item, "main");
      if (main_txt) {
        out.main = main_txt;
      }
      const char *desc = json_string_or(item, "description");
      if (desc) {
        out.description = desc;
      }
      icon_code = json_string_or(item, "icon");
    }
  }

  out.visibility = static_cast<uint16_t>(json_int_or(root, "visibility", out.visibility));
  out.observationTime = static_cast<time_t>(json_int_or(root, "dt", out.observationTime));
  out.timezone = json_int_or(root, "timezone", out.timezone);
  const char *name = json_string_or(root, "name");
  if (name) {
    out.cityName = name;
  }

  out.iconVariant = map_icon_variant(icon_code);
  if (icon_code) {
    out.iconId = icon_code;
  }

  cJSON_Delete(root);
  return !std::isnan(out.temperature);
}

static void reset_forecast_entries(ForecastEntry *entries, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    entries[i] = ForecastEntry();
  }
}

static bool parse_forecast(const char *json, ForecastEntry *entries, size_t count) {
  time_t now = time(nullptr);
  struct tm now_info;
  if (!localtime_r(&now, &now_info)) {
    return false;
  }

  cJSON *root = cJSON_Parse(json);
  if (!root) {
    return false;
  }

  cJSON *list = cJSON_GetObjectItemCaseSensitive(root, "list");
  if (!cJSON_IsArray(list)) {
    cJSON_Delete(root);
    return false;
  }

  reset_forecast_entries(entries, count);
  int list_size = cJSON_GetArraySize(list);
  for (int i = 0; i < list_size; ++i) {
    cJSON *item = cJSON_GetArrayItem(list, i);
    if (!cJSON_IsObject(item)) {
      continue;
    }

    time_t timestamp = static_cast<time_t>(json_int_or(item, "dt", 0));
    if (timestamp == 0) {
      continue;
    }

    struct tm ts_info;
    if (!localtime_r(&timestamp, &ts_info)) {
      continue;
    }

    int diff_days = ts_info.tm_yday - now_info.tm_yday;
    if (diff_days < 1 || diff_days > static_cast<int>(count)) {
      continue;
    }

    cJSON *main = cJSON_GetObjectItemCaseSensitive(item, "main");
    float temp_min = NAN;
    float temp_max = NAN;
    if (cJSON_IsObject(main)) {
      temp_min = json_number_or(main, "temp_min", temp_min);
      temp_max = json_number_or(main, "temp_max", temp_max);
    }

    int condition = 0;
    const char *icon_code = nullptr;
    cJSON *weather = cJSON_GetObjectItemCaseSensitive(item, "weather");
    if (cJSON_IsArray(weather) && cJSON_GetArraySize(weather) > 0) {
      cJSON *w0 = cJSON_GetArrayItem(weather, 0);
      if (cJSON_IsObject(w0)) {
        condition = json_int_or(w0, "id", 0);
        icon_code = json_string_or(w0, "icon");
      }
    }

    ForecastEntry &entry = entries[diff_days - 1];
    if (!entry.valid) {
      entry.minTemp = temp_min;
      entry.maxTemp = temp_max;
      entry.timestamp = timestamp;
      entry.conditionId = condition;
      entry.iconVariant = map_icon_variant(icon_code);
      if (icon_code) {
        entry.iconId = icon_code;
      }
      entry.valid = true;
      entry.middayOffset = static_cast<uint8_t>(abs(ts_info.tm_hour - 12));
    } else {
      if (temp_min < entry.minTemp) {
        entry.minTemp = temp_min;
      }
      if (temp_max > entry.maxTemp) {
        entry.maxTemp = temp_max;
      }
      uint8_t offset = static_cast<uint8_t>(abs(ts_info.tm_hour - 12));
      if (offset < entry.middayOffset) {
        entry.timestamp = timestamp;
        entry.conditionId = condition;
        entry.iconVariant = map_icon_variant(icon_code);
        if (icon_code) {
          entry.iconId = icon_code;
        }
        entry.middayOffset = offset;
      }
    }
  }

  cJSON_Delete(root);
  return true;
}

static void reset_minutely_entries(MinutelyEntry *entries, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    entries[i] = MinutelyEntry();
  }
}

static void reset_hourly_entries(HourlyEntry *entries, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    entries[i] = HourlyEntry();
  }
}

static bool parse_onecall(const char *json,
                          CurrentWeatherData &current,
                          ForecastEntry *daily,
                          size_t daily_count,
                          MinutelyEntry *minutely,
                          size_t minutely_count,
                          HourlyEntry *hourly,
                          size_t hourly_count) {
  cJSON *root = cJSON_Parse(json);
  if (!root) {
    return false;
  }

  cJSON *current_obj = cJSON_GetObjectItemCaseSensitive(root, "current");
  if (cJSON_IsObject(current_obj)) {
    current.temperature = json_number_or(current_obj, "temp", current.temperature);
    current.feelsLike = json_number_or(current_obj, "feels_like", current.feelsLike);
    current.pressure = json_number_or(current_obj, "pressure", current.pressure);
    current.humidity = static_cast<uint8_t>(json_int_or(current_obj, "humidity", current.humidity));
    current.observationTime = static_cast<time_t>(json_int_or(current_obj, "dt", current.observationTime));
    current.sunrise = static_cast<time_t>(json_int_or(current_obj, "sunrise", current.sunrise));
    current.sunset = static_cast<time_t>(json_int_or(current_obj, "sunset", current.sunset));
    cJSON *weather = cJSON_GetObjectItemCaseSensitive(current_obj, "weather");
    parse_weather_item(
        weather,
        &current.conditionId,
        &current.iconId,
        &current.iconVariant,
        &current.main,
        &current.description);
  }

  if (daily && daily_count > 0) {
    reset_forecast_entries(daily, daily_count);
    cJSON *daily_arr = cJSON_GetObjectItemCaseSensitive(root, "daily");
    if (cJSON_IsArray(daily_arr)) {
      int total = cJSON_GetArraySize(daily_arr);
      for (int i = 1; i < total && (size_t)(i - 1) < daily_count; ++i) {
        cJSON *item = cJSON_GetArrayItem(daily_arr, i);
        if (!cJSON_IsObject(item)) {
          continue;
        }
        ForecastEntry &entry = daily[i - 1];
        entry.valid = true;
        entry.timestamp = static_cast<time_t>(json_int_or(item, "dt", 0));
        cJSON *temp = cJSON_GetObjectItemCaseSensitive(item, "temp");
        if (cJSON_IsObject(temp)) {
          entry.minTemp = json_number_or(temp, "min", entry.minTemp);
          entry.maxTemp = json_number_or(temp, "max", entry.maxTemp);
        }
        cJSON *weather = cJSON_GetObjectItemCaseSensitive(item, "weather");
        parse_weather_item(weather, &entry.conditionId, &entry.iconId, &entry.iconVariant, nullptr, nullptr);
        entry.middayOffset = 0;
      }
    }
  }

  if (minutely && minutely_count > 0) {
    reset_minutely_entries(minutely, minutely_count);
    cJSON *min_arr = cJSON_GetObjectItemCaseSensitive(root, "minutely");
    if (cJSON_IsArray(min_arr)) {
      int total = cJSON_GetArraySize(min_arr);
      for (int i = 0; i < total && (size_t)i < minutely_count; ++i) {
        cJSON *item = cJSON_GetArrayItem(min_arr, i);
        if (!cJSON_IsObject(item)) {
          continue;
        }
        MinutelyEntry &entry = minutely[i];
        entry.valid = true;
        entry.timestamp = static_cast<time_t>(json_int_or(item, "dt", 0));
        entry.precipitation = json_number_or(item, "precipitation", entry.precipitation);
      }
    }
  }

  if (hourly && hourly_count > 0) {
    reset_hourly_entries(hourly, hourly_count);
    cJSON *hourly_arr = cJSON_GetObjectItemCaseSensitive(root, "hourly");
    if (cJSON_IsArray(hourly_arr)) {
      int total = cJSON_GetArraySize(hourly_arr);
      for (int i = 1; i < total && (size_t)(i - 1) < hourly_count; ++i) {
        cJSON *item = cJSON_GetArrayItem(hourly_arr, i);
        if (!cJSON_IsObject(item)) {
          continue;
        }
        HourlyEntry &entry = hourly[i - 1];
        entry.valid = true;
        entry.timestamp = static_cast<time_t>(json_int_or(item, "dt", 0));
        entry.temperature = json_number_or(item, "temp", entry.temperature);
        entry.feelsLike = json_number_or(item, "feels_like", entry.feelsLike);
        entry.pop = json_number_or(item, "pop", entry.pop);
        cJSON *rain = cJSON_GetObjectItemCaseSensitive(item, "rain");
        if (cJSON_IsObject(rain)) {
          entry.rain1h = json_number_or(rain, "1h", entry.rain1h);
        }
        cJSON *snow = cJSON_GetObjectItemCaseSensitive(item, "snow");
        if (cJSON_IsObject(snow)) {
          entry.snow1h = json_number_or(snow, "1h", entry.snow1h);
        }
        cJSON *weather = cJSON_GetObjectItemCaseSensitive(item, "weather");
        parse_weather_item(weather, &entry.conditionId, &entry.iconId, &entry.iconVariant, nullptr, nullptr);
      }
    }
  }

  cJSON_Delete(root);
  return !std::isnan(current.temperature);
}
}  // namespace

WeatherFetcher::WeatherFetcher() = default;

esp_err_t WeatherFetcher::fetchUrl(const char *url, std::string &out) {
  lastError_.clear();
  out.clear();
  if (!url || !*url) {
    lastError_ = "URL invalide";
    return ESP_ERR_INVALID_ARG;
  }

  esp_http_client_config_t config = {};
  config.url = url;
  config.timeout_ms = timeout_ms_;
  config.skip_cert_common_name_check = true;
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
  config.crt_bundle_attach = esp_crt_bundle_attach;
#endif

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client) {
    lastError_ = "esp_http_client_init a echoue";
    return ESP_FAIL;
  }

  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK) {
    lastError_ = "esp_http_client_open a echoue";
    esp_http_client_cleanup(client);
    return err;
  }

  int status = esp_http_client_fetch_headers(client);
  if (status < 0) {
    lastError_ = "HTTP headers invalides";
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return ESP_FAIL;
  }

  int http_code = esp_http_client_get_status_code(client);
  if (http_code != 200) {
    lastError_ = "HTTP code " + std::to_string(http_code);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return ESP_FAIL;
  }

  char buffer[512];
  int read_len = 0;
  while ((read_len = esp_http_client_read(client, buffer, sizeof(buffer))) > 0) {
    out.append(buffer, static_cast<size_t>(read_len));
  }

  esp_http_client_close(client);
  esp_http_client_cleanup(client);

  if (read_len < 0) {
    lastError_ = "Lecture HTTP incomplete";
    return ESP_FAIL;
  }

  return ESP_OK;
}

esp_err_t WeatherFetcher::fetchCurrent(const char *url, CurrentWeatherData &out) {
  std::string payload;
  esp_err_t err = fetchUrl(url, payload);
  if (err != ESP_OK) {
    ESP_LOGE("WeatherFetcher", "Fetch current failed: %s", lastError_.c_str());
    return err;
  }
  if (!parse_current(payload.c_str(), out)) {
    lastError_ = "Parsing current incomplet";
    return ESP_FAIL;
  }
  return ESP_OK;
}

esp_err_t WeatherFetcher::fetchForecast(const char *url, ForecastEntry *out, size_t count) {
  if (!out || count == 0) {
    lastError_ = "Buffer forecast invalide";
    return ESP_ERR_INVALID_ARG;
  }
  std::string payload;
  esp_err_t err = fetchUrl(url, payload);
  if (err != ESP_OK) {
    ESP_LOGE("WeatherFetcher", "Fetch forecast failed: %s", lastError_.c_str());
    return err;
  }
  if (!parse_forecast(payload.c_str(), out, count)) {
    lastError_ = "Parsing forecast incomplet";
    return ESP_FAIL;
  }
  return ESP_OK;
}

esp_err_t WeatherFetcher::fetchOneCall(const char *url,
                                       CurrentWeatherData &current,
                                       ForecastEntry *out,
                                       size_t count,
                                       MinutelyEntry *minutely,
                                       size_t minutely_count,
                                       HourlyEntry *hourly,
                                       size_t hourly_count) {
  std::string payload;
  esp_err_t err = fetchUrl(url, payload);
  if (err != ESP_OK) {
    ESP_LOGE("WeatherFetcher", "Fetch onecall failed: %s", lastError_.c_str());
    return err;
  }
  if (!parse_onecall(payload.c_str(), current, out, count, minutely, minutely_count, hourly, hourly_count)) {
    lastError_ = "Parsing onecall incomplet";
    return ESP_FAIL;
  }
  return ESP_OK;
}

bool WeatherFetcher::parseCurrentJson(const char* json, CurrentWeatherData& out) {
  return parse_current(json, out);
}

bool WeatherFetcher::parseForecastJson(const char* json, ForecastEntry* out, size_t count) {
  return parse_forecast(json, out, count);
}

bool WeatherFetcher::parseOneCallJson(const char* json,
                                      CurrentWeatherData& current,
                                      ForecastEntry* out,
                                      size_t count,
                                      MinutelyEntry* minutely,
                                      size_t minutely_count,
                                      HourlyEntry* hourly,
                                      size_t hourly_count) {
  return parse_onecall(json, current, out, count, minutely, minutely_count, hourly, hourly_count);
}
