# Main Functions Overview

This document lists the main runtime functions used by the weather station app.

## System and UI
- `app_main()` / `setup()` (`src/weatherStation.c`): boot sequence, HW init, UI start.
- `bsp_display_start_with_config()` (`src/esp_bsp.c`): LCD/QSPI init + LVGL port.
- `lvgl_port_init()` (`src/lv_port.c`): LVGL task/tick initialization.
- `ui_init()` (`src/ui/ui.c`): build EEZ UI objects.
- `ui_screen_start()` (`src/ui_screen.c`): start clock and UI timers.
- `lv_fs_spiffs_init()` (`src/lv_fs_spiffs.c`): register LVGL FS driver for `s:`.

## Weather Service
- `weather_service_start()` (`src/weather_service.cpp`): periodic weather updates + IP event hook.
- `weather_fetch_once()` (`src/weather_service.cpp`): fetch and apply current data to UI.

## Weather Fetcher (OWM)
- `WeatherFetcher::fetchCurrent()` (`libraries/weather/weather_fetcher.cpp`): OWM v2.5 current weather.
- `WeatherFetcher::fetchForecast()` (`libraries/weather/weather_fetcher.cpp`): OWM v2.5 forecast.
- `WeatherFetcher::fetchOneCall()` (`libraries/weather/weather_fetcher.cpp`): OWM One Call v3 (RAM only).

## Icon Lookup and Rendering
- `WeatherIconFindOffsetStream()` (`libraries/weather/weather_icons.cpp`): find icon by code/variant in `.bin` index.
- `weather_icons_set_main()` (`src/weather_icons.c`): decode + display main icon on `ui_meteo_img`.
- `svg2bin_decode_entry_at_offset()` (`src/svg2bin_decoder.c`): decode RGB565 entry at offset.

## UI Data Binding
- `set_var_ui_meteo_*()` (`src/vars.c`): update UI-bound variables.
