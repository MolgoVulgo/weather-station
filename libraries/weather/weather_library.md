# Bibliotheque weather_XX

Ce document couvre uniquement la bibliotheque meteo (fetch + icones).
Il ne traite pas des scripts ou outils externes.

## Modules et fichiers
- `include/weather_fetcher.h` + `src/weather_fetcher.cpp`
- `include/weather_icons.h` + `src/weather_icons.cpp` + `src/weather_icons_data.cpp`

## A quoi ca sert
- Recuperer la meteo courante et les previsions via OpenWeatherMap.
- Exposer des structures C++ simples a exploiter.
- Mapper le `conditionId` + `iconId` (OWM) vers un index integre dans un fichier `.bin`.

## Dependances
- `esp_http_client` + mbedTLS (HTTP/HTTPS).
- `cJSON` (parsing JSON).
- `ESP-IDF` + `time.h`.
- `svg2bin_decoder` (lookup icones via index integre).

## Schema de data

### CurrentWeatherData
```
CurrentWeatherData
├─ temperature : float
├─ feelsLike : float
├─ tempMin / tempMax : float
├─ pressure / pressureSeaLevel / pressureGroundLevel : float
├─ humidity : uint8_t
├─ visibility : uint16_t
├─ windKmh / windDeg / windGustKmh : float
├─ clouds : uint8_t
├─ rain1h / rain3h : float
├─ snow1h / snow3h : float
├─ observationTime : time_t
├─ sunrise / sunset : time_t
├─ timezone : int32_t
├─ cityName / country : String
├─ main / description : String
├─ iconId : String           // code OWM ("01d", "04n", ...)
├─ iconVariant : uint8_t     // 0=jour, 1=nuit, 2=neutre
└─ conditionId : int
```

### ForecastEntry
```
ForecastEntry
├─ timestamp : time_t
├─ minTemp / maxTemp : float
├─ iconId : String
├─ iconVariant : uint8_t
├─ conditionId : int
├─ valid : bool
└─ middayOffset : uint8_t
```

### MinutelyEntry
```
MinutelyEntry
├─ timestamp : time_t
├─ precipitation : float
└─ valid : bool
```

### HourlyEntry
```
HourlyEntry
├─ timestamp : time_t
├─ temperature : float
├─ feelsLike : float
├─ pop : float
├─ rain1h / snow1h : float
├─ iconId : String
├─ iconVariant : uint8_t
├─ conditionId : int
└─ valid : bool
```

### WeatherIconRef
```
WeatherIconRef
├─ code : uint16_t
└─ variant : uint8_t
```

## Schema d'appel

### Meteo courante
```
WeatherFetcher::fetchCurrent(url, out)
  -> HTTPClient.begin(client, url)
  -> HTTP GET
  -> cJSON parse buffer
  -> remplit CurrentWeatherData
  -> map conditionId + iconId (jour/nuit) vers un index d'icone
```

### Previsions
```
WeatherFetcher::fetchForecast(url, out[], count)
  -> HTTPClient.begin(client, url)
  -> HTTP GET
  -> cJSON parse buffer
  -> regroupe par jour
  -> min/max + iconId (entree la plus proche de midi)
```

### One Call (RAM)
```
WeatherFetcher::fetchOneCall(url, current, daily[], count, minutely[], mcount, hourly[], hcount)
  -> HTTPClient.begin(client, url)
  -> HTTP GET
  -> cJSON parse buffer
  -> remplit CurrentWeatherData + daily/minutely/hourly
  -> pas de cache fichier (RAM uniquement)
```

### One Call (RAM)
```
WeatherFetcher::fetchOneCall(url, current, daily[], count, minutely[], mcount, hourly[], hcount)
  -> HTTPClient.begin(client, url)
  -> HTTP GET
  -> cJSON parse buffer
  -> remplit CurrentWeatherData + daily/minutely/hourly
  -> pas de cache fichier (RAM uniquement)
```

Note: utiliser `OPENWEATHERMAP_API_KEY_3` pour One Call v3 et `OPENWEATHERMAP_API_KEY_2` pour v2.5. Le mode est exclusif (v3 ou v2.5), jamais hybride.

### Recherche d'icone
```
WeatherIconFindOffsetStream(fp, conditionId, iconId)
  -> lookup offset via index integre du .bin
```

## Exemple minimal
```cpp
WeatherFetcher fetcher;
CurrentWeatherData current;

if (fetcher.fetchCurrent(buildCurrentWeatherUrl(), current) == ESP_OK) {
  FILE *fp = fopen("/spiffs/icon_150.bin", "rb");
  uint32_t offset = 0;
  if (fp && WeatherIconFindOffsetStream(fp, current.conditionId, current.iconId.c_str(), &offset) == ESP_OK) {
    // svg2bin_decode_entry_at_offset(fp, offset, ...)
  }
  if (fp) fclose(fp);
} else {
  printf("%s\n", fetcher.lastError().c_str());
}
```
