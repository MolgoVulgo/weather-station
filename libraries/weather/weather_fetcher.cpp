#include "weather_fetcher.h"

#include <ESP8266HTTPClient.h>
#include <JsonListener.h>
#include <JsonStreamingParser.h>

namespace {
constexpr float MS_TO_KMH = 3.6f;

struct IconMapEntry {
  const char* code;
  const char* assetId;
};

constexpr IconMapEntry kIconMap[] = {
    {"01d", "day_clear"},
    {"01n", "night_clear"},
    {"02d", "day_partial_cloud"},
    {"02n", "night_partial_cloud"},
    {"03d", "cloudy"},
    {"03n", "cloudy"},
    {"04d", "overcast"},
    {"04n", "overcast"},
    {"09d", "rain"},
    {"09n", "rain"},
    {"10d", "day_rain"},
    {"10n", "night_rain"},
    {"11d", "day_rain_thunder"},
    {"11n", "night_rain_thunder"},
    {"13d", "day_snow"},
    {"13n", "night_snow"},
    {"50d", "mist"},
    {"50n", "mist"},
};

const char* mapIconName(const String& iconCode, int conditionId) {
  if (iconCode.length() >= 3) {
    for (const auto& entry : kIconMap) {
      if (iconCode == entry.code) {
        return entry.assetId;
      }
    }
  }

  // Fallback par conditionId si le code icone est absent.
  if (conditionId >= 200 && conditionId <= 232) return "day_rain_thunder";
  if (conditionId >= 300 && conditionId <= 321) return "rain";
  if (conditionId >= 500 && conditionId <= 504) return "day_rain";
  if (conditionId == 511) return "day_snow";
  if (conditionId >= 520 && conditionId <= 531) return "rain";
  if (conditionId >= 600 && conditionId <= 622) return "day_snow";
  if (conditionId >= 701 && conditionId <= 781) return "mist";
  if (conditionId == 800) return "day_clear";
  if (conditionId == 801) return "day_partial_cloud";
  if (conditionId == 802) return "cloudy";
  if (conditionId >= 803 && conditionId <= 804) return "overcast";
  return "day_clear";
}

class CurrentWeatherParser : public JsonListener {
 public:
  explicit CurrentWeatherParser(CurrentWeatherData& data) : data_(data) {}

  bool ok() const { return hasTemp_; }

  void whitespace(char) override {}
  void startDocument() override {}
  void endDocument() override {}
  void startArray() override {}
  void endArray() override {}
  void startObject() override { currentParent_ = currentKey_; }
  void endObject() override {
    if (currentParent_ == "weather") {
      weatherItemCounter_++;
    }
    currentParent_ = "";
  }

  void key(String key) override { currentKey_ = key; }

  void value(String value) override {
    if (currentParent_ == "main") {
      if (currentKey_ == "temp") {
        data_.temperature = value.toFloat();
        hasTemp_ = true;
      } else if (currentKey_ == "feels_like") {
        data_.feelsLike = value.toFloat();
      } else if (currentKey_ == "temp_min") {
        data_.tempMin = value.toFloat();
      } else if (currentKey_ == "temp_max") {
        data_.tempMax = value.toFloat();
      } else if (currentKey_ == "pressure") {
        data_.pressure = value.toFloat();
      } else if (currentKey_ == "sea_level") {
        data_.pressureSeaLevel = value.toFloat();
      } else if (currentKey_ == "grnd_level") {
        data_.pressureGroundLevel = value.toFloat();
      } else if (currentKey_ == "humidity") {
        data_.humidity = static_cast<uint8_t>(value.toInt());
      }
      return;
    }

    if (currentParent_ == "wind") {
      if (currentKey_ == "speed") {
        data_.windKmh = value.toFloat() * MS_TO_KMH;
      } else if (currentKey_ == "deg") {
        data_.windDeg = value.toFloat();
      } else if (currentKey_ == "gust") {
        data_.windGustKmh = value.toFloat() * MS_TO_KMH;
      }
      return;
    }

    if (currentParent_ == "clouds" && currentKey_ == "all") {
      data_.clouds = static_cast<uint8_t>(value.toInt());
      return;
    }

    if (currentParent_ == "rain") {
      if (currentKey_ == "1h") {
        data_.rain1h = value.toFloat();
      } else if (currentKey_ == "3h") {
        data_.rain3h = value.toFloat();
      }
      return;
    }

    if (currentParent_ == "snow") {
      if (currentKey_ == "1h") {
        data_.snow1h = value.toFloat();
      } else if (currentKey_ == "3h") {
        data_.snow3h = value.toFloat();
      }
      return;
    }

    if (currentParent_ == "sys") {
      if (currentKey_ == "sunrise") {
        data_.sunrise = value.toInt();
      } else if (currentKey_ == "sunset") {
        data_.sunset = value.toInt();
      } else if (currentKey_ == "country") {
        data_.country = value;
      }
      return;
    }

    if (currentParent_ == "weather" && weatherItemCounter_ == 0) {
      if (currentKey_ == "id") {
        data_.conditionId = value.toInt();
      } else if (currentKey_ == "main") {
        data_.main = value;
      } else if (currentKey_ == "description") {
        data_.description = value;
      } else if (currentKey_ == "icon") {
        iconCode_ = value;
      }
      if (!iconCode_.isEmpty() && data_.conditionId != 0) {
        data_.iconId = mapIconName(iconCode_, data_.conditionId);
      }
      return;
    }

    if (currentKey_ == "visibility") {
      data_.visibility = static_cast<uint16_t>(value.toInt());
      return;
    }

    if (currentKey_ == "dt") {
      data_.observationTime = static_cast<time_t>(value.toInt());
      return;
    }

    if (currentKey_ == "timezone") {
      data_.timezone = value.toInt();
      return;
    }

    if (currentKey_ == "name") {
      data_.cityName = value;
      return;
    }
  }

 private:
  CurrentWeatherData& data_;
  String currentKey_;
  String currentParent_;
  String iconCode_;
  uint8_t weatherItemCounter_ = 0;
  bool hasTemp_ = false;
};

class ForecastWeatherParser : public JsonListener {
 public:
  ForecastWeatherParser(ForecastEntry* entries, size_t count)
      : entries_(entries), count_(count) {
    resetItems();
    time_t now = time(nullptr);
    struct tm nowInfo;
    if (localtime_r(&now, &nowInfo)) {
      todayDayOfYear_ = nowInfo.tm_yday;
      hasToday_ = true;
    }
  }

  bool ok() const { return hasToday_; }

  void whitespace(char) override {}
  void startDocument() override {}
  void endDocument() override {}

  void startArray() override {
    arrayDepth_++;
    if (currentKey_ == "list") {
      inList_ = true;
      listDepth_ = arrayDepth_;
    } else if (currentKey_ == "weather") {
      inWeather_ = true;
      weatherDepth_ = arrayDepth_;
    }
  }

  void endArray() override {
    if (inWeather_ && arrayDepth_ == weatherDepth_) {
      inWeather_ = false;
      weatherDepth_ = -1;
    }
    if (inList_ && arrayDepth_ == listDepth_) {
      inList_ = false;
      listDepth_ = -1;
    }
    arrayDepth_--;
  }

  void startObject() override { currentParent_ = currentKey_; }
  void endObject() override {
    if (currentParent_ == "weather") {
      weatherItemCounter_++;
    }
    currentParent_ = "";
  }

  void key(String key) override { currentKey_ = key; }

  void value(String value) override {
    if (!inList_ || !hasToday_) {
      return;
    }
    if (currentKey_ == "dt") {
      itemTimestamp_ = value.toInt();
      return;
    }
    if (currentParent_ == "main") {
      if (currentKey_ == "temp_min") {
        itemTempMin_ = value.toFloat();
      } else if (currentKey_ == "temp_max") {
        itemTempMax_ = value.toFloat();
      }
      return;
    }
    if (inWeather_ && weatherItemCounter_ == 0) {
      if (currentKey_ == "id") {
        itemWeatherId_ = value.toInt();
      } else if (currentKey_ == "icon") {
        itemIconCode_ = value;
      }
      return;
    }
    if (currentKey_ == "dt_txt") {
      commitItem();
    }
  }

 private:
  void resetItems() {
    for (size_t i = 0; i < count_; ++i) {
      entries_[i] = ForecastEntry();
    }
  }

  void commitItem() {
    if (itemTimestamp_ == 0) {
      resetItemFields();
      return;
    }
    struct tm tsInfo;
    if (!localtime_r(&itemTimestamp_, &tsInfo)) {
      resetItemFields();
      return;
    }
    int diffDays = tsInfo.tm_yday - todayDayOfYear_;
    if (diffDays < 1 || diffDays > static_cast<int>(count_)) {
      resetItemFields();
      return;
    }

    ForecastEntry& entry = entries_[diffDays - 1];
    if (!entry.valid) {
      entry.minTemp = itemTempMin_;
      entry.maxTemp = itemTempMax_;
      entry.timestamp = itemTimestamp_;
      entry.conditionId = itemWeatherId_;
      entry.iconId = mapIconName(itemIconCode_, entry.conditionId);
      entry.valid = true;
      entry.middayOffset = static_cast<uint8_t>(abs(tsInfo.tm_hour - 12));
    } else {
      if (itemTempMin_ < entry.minTemp) {
        entry.minTemp = itemTempMin_;
      }
      if (itemTempMax_ > entry.maxTemp) {
        entry.maxTemp = itemTempMax_;
      }
      uint8_t offset = static_cast<uint8_t>(abs(tsInfo.tm_hour - 12));
      if (offset < entry.middayOffset) {
        entry.timestamp = itemTimestamp_;
        entry.conditionId = itemWeatherId_;
        entry.iconId = mapIconName(itemIconCode_, entry.conditionId);
        entry.middayOffset = offset;
      }
    }
    resetItemFields();
  }

  void resetItemFields() {
    itemTimestamp_ = 0;
    itemTempMin_ = NAN;
    itemTempMax_ = NAN;
    itemWeatherId_ = 0;
    itemIconCode_ = "";
    weatherItemCounter_ = 0;
  }

  ForecastEntry* entries_;
  size_t count_;
  bool hasToday_ = false;
  int todayDayOfYear_ = 0;

  String currentKey_;
  String currentParent_;

  bool inList_ = false;
  bool inWeather_ = false;
  int arrayDepth_ = 0;
  int listDepth_ = -1;
  int weatherDepth_ = -1;
  uint8_t weatherItemCounter_ = 0;

  time_t itemTimestamp_ = 0;
  float itemTempMin_ = NAN;
  float itemTempMax_ = NAN;
  int itemWeatherId_ = 0;
  String itemIconCode_;
};
}  // namespace

WeatherFetcher::WeatherFetcher(BearSSL::WiFiClientSecure& client) : client_(client) {}

bool WeatherFetcher::fetchCurrent(const String& url, CurrentWeatherData& out) {
  lastError_ = "";
  HTTPClient http;
  if (!http.begin(client_, url)) {
    lastError_ = "HTTPClient.begin a echoue";
    return false;
  }
  const int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    lastError_ = "HTTP code " + String(httpCode);
    http.end();
    return false;
  }

  CurrentWeatherParser listener(out);
  JsonStreamingParser parser;
  parser.setListener(&listener);
  Stream& stream = http.getStream();
  while (stream.available()) {
    parser.parse(static_cast<char>(stream.read()));
    yield();
  }
  http.end();
  if (!listener.ok()) {
    lastError_ = "Parsing current incomplet";
    return false;
  }
  if (out.iconId.isEmpty()) {
    out.iconId = mapIconName("", out.conditionId);
  }
  return true;
}

bool WeatherFetcher::fetchForecast(const String& url, ForecastEntry* out, size_t count) {
  lastError_ = "";
  HTTPClient http;
  if (!http.begin(client_, url)) {
    lastError_ = "HTTPClient.begin a echoue";
    return false;
  }
  const int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    lastError_ = "HTTP code " + String(httpCode);
    http.end();
    return false;
  }

  ForecastWeatherParser listener(out, count);
  JsonStreamingParser parser;
  parser.setListener(&listener);
  Stream& stream = http.getStream();
  while (stream.available()) {
    parser.parse(static_cast<char>(stream.read()));
    yield();
  }
  http.end();
  if (!listener.ok()) {
    lastError_ = "Parsing forecast incomplet";
    return false;
  }
  return true;
}
