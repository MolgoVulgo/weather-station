# OWM API v3 - Format du JSON (exemple)

Ce document décrit la structure du fichier `docs/owm-api-3.json` (réponse type OpenWeatherMap One Call v3) tel qu'il apparaît dans ce projet. Il sert de référence pour parser et exploiter les données météo.

## Vue d'ensemble
- **Format**: JSON
- **Racine**: objet avec coordonnées, fuseau, et blocs météo (current, minutely, hourly, daily, alerts).
- **Timestamps**: champs `dt`, `sunrise`, `sunset`, etc. sont des **Unix time** en **secondes**.
- **Timezone**: `timezone` (ex: `Europe/Paris`) et `timezone_offset` (secondes).

## Champs racine
- `lat` (number): latitude.
- `lon` (number): longitude.
- `timezone` (string): identifiant IANA du fuseau horaire.
- `timezone_offset` (int): décalage en secondes par rapport à UTC.
- `current` (object): conditions actuelles.
- `minutely` (array): précipitations minute par minute.
- `hourly` (array): prévisions horaires.
- `daily` (array): prévisions journalières.
- `alerts` (array): alertes météo (optionnel selon la zone).

## Objet `current`
Champs observés dans l'exemple:
- `dt`: timestamp de la mesure.
- `sunrise`, `sunset`: timestamps du lever/coucher du soleil.
- `temp`: température.
- `feels_like`: température ressentie.
- `pressure`: pression atmosphérique.
- `humidity`: humidité relative (%).
- `dew_point`: point de rosée.
- `uvi`: index UV.
- `clouds`: couverture nuageuse (%).
- `visibility`: visibilité (m).
- `wind_speed`: vitesse du vent.
- `wind_deg`: direction du vent (degrés).
- `wind_gust`: rafales.
- `weather`: tableau d'objets météo (voir plus bas).

## Tableau `minutely`
Chaque entrée contient:
- `dt`: timestamp (minute).
- `precipitation`: précipitations (mm, selon l'API OpenWeatherMap).

## Tableau `hourly`
Chaque entrée contient:
- `dt`: timestamp horaire.
- `temp`, `feels_like`, `pressure`, `humidity`, `dew_point`, `uvi`, `clouds`, `visibility`.
- `wind_speed`, `wind_deg`, `wind_gust`.
- `weather`: tableau d'objets météo.
- `pop`: probabilité de précipitation (0-1).
- `rain.1h`, `snow.1h`: cumul 1h (mm) lorsque présent.

### Usage dans l'UI (strip horaire)
- Le point de départ est **toujours** `json.hourly[0]` (son `dt` alimente le 1er slot horaire).
- L'UI affiche **7 slots**: `hourly[0..6]`.
- Mapping des timestamps:
  - `hourly[2..6]` prennent leurs `dt` depuis `json.hourly[0..4]`.
  - `hourly[0]` et `hourly[1]` sont des valeurs d'historique (décalage: `hourly[2] -> hourly[1] -> hourly[0]`).
- A l'init de l'UI, `hourly[0]` et `hourly[1]` reprennent les valeurs de `hourly[2]`.
- Si les données hourly sont absentes/invalides, l'UI conserve l'affichage actuel et un log debug est émis.

## Tableau `daily`
Chaque entrée contient:
- `dt`, `sunrise`, `sunset`.
- `moonrise`, `moonset`, `moon_phase`.
- `summary`: résumé textuel.
- `temp`: objet (day, min, max, night, eve, morn).
- `feels_like`: objet (day, night, eve, morn).
- `pressure`, `humidity`, `dew_point`.
- `wind_speed`, `wind_deg`, `wind_gust`.
- `weather`: tableau d'objets météo.
- `clouds`.
- `pop`: probabilité de précipitation (0-1).
- `rain`, `snow`: cumul (mm) lorsque présent.
- `uvi`: index UV.

## Tableau `alerts`
Chaque entrée contient:
- `sender_name`: organisme émetteur.
- `event`: nom de l'alerte.
- `start`, `end`: timestamps.
- `description`: description textuelle.
- `tags`: tableau de catégories.

## Objet `weather`
Commun à `current`, `hourly`, `daily`:
- `id` (int): code météo OpenWeatherMap.
- `main` (string): groupe météo (ex: Clouds, Rain).
- `description` (string): description locale.
- `icon` (string): code d'icône.

## Remarques
- Les unités dépendent des paramètres de l'appel OWM (metric/imperial/standard). Dans l'exemple, les températures autour de 4-6 suggèrent des °C (metric).
- Le tableau `alerts` peut être absent si aucune alerte n'est active.
- `rain` / `snow` n'apparaissent que si pertinent.
