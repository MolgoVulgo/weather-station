# Bibliotheque weather_XX

Ce document couvre uniquement la bibliotheque meteo (fetch + icones).
Il ne traite pas des scripts ou outils externes.

## Modules et fichiers
- `include/weather_fetcher.h` + `src/weather_fetcher.cpp`
- `include/weather_icons.h` + `src/weather_icons.cpp` + `src/weather_icons_data.cpp`

## A quoi ca sert
- Recuperer la meteo courante et les previsions via OpenWeatherMap.
- Exposer des structures C++ simples a exploiter.
- Mapper un `iconId` vers un binaire RGB565 stocke en LittleFS.

## Dependances
- `ESP8266HTTPClient` (requete HTTP/HTTPS).
- `BearSSL::WiFiClientSecure` (TLS).
- `JsonStreamingParser` (parsing JSON en streaming).
- `Arduino` + `time.h`.
- `LittleFS` (lecture d'icones).

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
├─ iconId : String
└─ conditionId : int
```

### ForecastEntry
```
ForecastEntry
├─ timestamp : time_t
├─ minTemp / maxTemp : float
├─ iconId : String
├─ conditionId : int
├─ valid : bool
└─ middayOffset : uint8_t
```

### WeatherIconAsset
```
WeatherIconAsset
├─ id : const char*
├─ width / height : uint16_t
└─ filePath : const char*   // chemin LittleFS
```

## Schema d'appel

### Meteo courante
```
WeatherFetcher::fetchCurrent(url, out)
  -> HTTPClient.begin(client, url)
  -> HTTP GET
  -> JsonStreamingParser + CurrentWeatherParser
  -> remplit CurrentWeatherData
  -> map iconId (code OWM -> asset id)
```

### Previsions
```
WeatherFetcher::fetchForecast(url, out[], count)
  -> HTTPClient.begin(client, url)
  -> HTTP GET
  -> JsonStreamingParser + ForecastWeatherParser
  -> regroupe par jour
  -> min/max + iconId (entree la plus proche de midi)
```

### Recherche d'icone
```
FindWeatherIconAsset(iconId)
  -> parcours kWeatherIconAssets
  -> retourne pointeur sur WeatherIconAsset ou nullptr
```

## Exemple minimal
```cpp
WiFiClientSecure client;
client.setInsecure();

WeatherFetcher fetcher(client);
CurrentWeatherData current;

if (fetcher.fetchCurrent(buildCurrentWeatherUrl(), current)) {
  const WeatherIconAsset* icon = FindWeatherIconAsset(current.iconId);
  if (icon) {
    // Lire icon->filePath via LittleFS et afficher.
  }
} else {
  Serial.println(fetcher.lastError());
}
```
