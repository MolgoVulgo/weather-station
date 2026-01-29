# Technical Documentation - Weather-Station (LVGL demo)

## Purpose
This project is a PlatformIO/ESP-IDF build for the JC3248W535EN board (320x480 display). The code starts LVGL 8.4 with an AXS15231B LCD driver over QSPI and an associated touch controller.

## Source Structure
- `src/weatherStation.c`: entry point `app_main()` and LVGL startup sequence + SD init.
- `src/esp_bsp.c` / `src/esp_bsp.h`: board BSP, QSPI/I2C init, LCD, touch, backlight, tear sync.
- `src/lv_port.c` / `src/lv_port.h`: LVGL port (task, tick, buffers, flush, rotation, input).
- `src/display.h`: simple LCD API without LVGL.
- `src/esp_lcd_axs15231b.c` / `src/esp_lcd_axs15231b.h`: low-level AXS15231B driver.
- `src/esp_lcd_touch.c` / `src/esp_lcd_touch.h`: ESP LCD touch abstraction.
- `src/lv_conf.h`: LVGL compile-time configuration.
- `src/ui/`: EEZ Studio export (UI, assets, screens). Do not edit manually.
- `libraries/weather/`: ESP-IDF weather fetcher and OWM icon mapping (uses embedded icon index).
- `src/hourly_strip.cpp` / `src/hourly_strip.h`: hourly strip controller (7 icons, animation, local buffer).

## UI: EEZ Studio
- `ui/` (EEZ Studio): generates an `objects` structure (e.g. `objects.ui_screen_label_time`) and an EEZ screen pipeline (`loadScreen`, `tick_screen`).
- Do not modify `src/ui/` contents (generated files).

## UI Variables (EEZ)
- Variables declared in `src/ui/vars.h` must be implemented in `src/vars.c` via `get_var_` / `set_var_` (generate/update `src/vars.c` whenever `src/ui/vars.h` changes).
- The UI updates labels in `src/ui/screens.c` on each `tick_screen()` by reading these getters.

## UI: meteo details (hourly strip)
- The `ui_detail_hourly` container shows 7 widgets (slots) centered on a vertical axis. Each widget is 50 px wide.
- Time scale: 1 hour = 50 px. The visual offset moves continuously based on `tm_min`/`tm_sec`.
- Hourly data comes from One Call v3 (`hourly` array, 12 entries loaded into `hourly_cache`).
- Logical mapping: slot 2 = "now" (`hourly[0]`), slots 3..6 = +1..+4 (`hourly[1..4]`), slots 0..1 = -2..-1 (history).
- On first display, slots 0..1 reuse slot 2 if no history is available.
- Animation: offset moves continuously; during sliding, an extra slot is added after the last one to show +5h (`hourly[5]`).
- Logical shift: on hour change, data slides by one slot and the offset resets to 0.
  The incoming hour data is already known (e.g. `json.hourly[4]` in the weather service layer).

## Internationalization (i18n)
- Translations are defined in `src/i18n.c` and resolved via the `_()` macro from `src/lv_i18n.h`.
- The selected language is stored in NVS (`lang_cfg`) through `src/lanague.c` and synced with the UI `ui_setting_laguage` dropdown.
- Forecast day labels use `app_i18n_weekday_short()` and OpenWeatherMap requests use `lanague_get_weather_code()` for `lang=`.

## Boot Flow
1. `app_main()` calls `setup()` in `src/weatherStation.c`. (default skeleton)
2. HW logs (chip, flash, heap, PSRAM).
3. Create `bsp_display_cfg_t` (buffer size, rotation).
4. `bsp_display_start_with_config()`:
   - `lvgl_port_init()` creates the LVGL timer + LVGL task.
   - init backlight PWM.
   - init QSPI LCD + AXS15231B panel.
   - init I2C touch and register LVGL input.
5. `ui_init()` creates the LVGL UI and shows `ui_start` with progress.
6. Wi-Fi init (STA) with NVS credentials (defaults from `include/secrets.h`), wait for IP.
   - If no credentials or repeated failures, start captive portal AP `StationMeteo` (password `stationmeteo`) and show `ui_wifi`.
   - The portal scan endpoint logs the SSID list (with RSSI) and the UI offers a refresh button to re-run the scan.
7. NTP sync (3 attempts); logs error if it fails.
8. SPIFFS mount + LVGL FS driver registration.
9. SD mount (`/sdcard`) and list files.
10. `ui_screen_start()` starts the local clock (LVGL timer).
11. Weather service runs current + forecast, then switches to `ui_meteo`.

## Display Pipeline
- `bsp_display_new()` configures the QSPI bus and the AXS15231B panel.
- The panel init sequence is in `lcd_init_cmds[]` (`src/esp_bsp.c`).
- `lvgl_port_add_disp()` allocates buffers and registers the LVGL driver.
- `lvgl_port_flush_callback()` transfers pixels to the panel, with software rotation and a transport buffer.
- Tear sync (optional) uses a GPIO interrupt and semaphores (see `bsp_display_sync_task`).

## Touch
- I2C init via `bsp_i2c_init()`.
- `bsp_touch_new()` creates the AXS15231B touch handle.
- Coordinates are corrected based on rotation in `bsp_touch_process_points_cb()`.

## Key Configuration
- Rotation: `LVGL_PORT_ROTATION_DEGREE` in `src/weatherStation.c`.
- LVGL: `src/lv_conf.h`, injected via `LV_CONF_PATH` in `platformio.ini`.
- LVGL buffer size: `bsp_display_cfg_t.buffer_size` in `src/weatherStation.c`.
- QSPI/I2C pins: `src/esp_bsp.h`.
- NTP: pool and offset (seconds) in NVS, namespace `time_cfg` (`src/time_sync.c`).
- Time format: `hour_format` in NVS `time_cfg` (24h default, 12h shows `hh:mm am/pm`).
- Temperature unit: `temp_unit` in NVS `weather_cfg` (0=°C, 1=°F).
- Wi-Fi: SSID + password in NVS, namespace `wifi_cfg` (`src/wifi_manager.c`).
- Wi-Fi reset: set `WIFI_RESET_NVS=1` in `include/secrets.h` to clear stored credentials.

## Extension Points
- Use `bsp_display_lock()` / `bsp_display_unlock()` to protect LVGL calls from other tasks.
- Adjust `buffer_size`, `trans_size`, and rotation to optimize performance/memory.
- Add sensors/metrics in `setup()` before or after LVGL startup.

### Using EEZ Studio (`ui/`)
1. Keep `#include "ui_backend.h"` in `src/weatherStation.c` and `src/ui_screen.c`.
2. The UI backend is fixed to EEZ (no build flag switch).
3. Verify `src/CMakeLists.txt`:
   - `INCLUDE_DIRS` contains `src/ui`

## Setting Up a UI from IDE-Generated Sources

### EEZ Studio -> `src/ui/`
1. Export to LVGL/C from EEZ Studio.
2. Copy the generated files into `src/ui/`.
3. Verify that the `objects` structure exposes the right IDs (e.g. `objects.ui_screen_label_time`).
4. Adjust application code to EEZ object names if needed.
5. Build and verify.

## Key Functions
- `sdcard_mount()` (`src/weatherStation.c`): mounts SD card in SDMMC 1-bit mode on `/sdcard` (CMD=GPIO11, CLK=GPIO12, D0=GPIO13).
- `sdcard_list_dir()` (`src/weatherStation.c`): logs SD contents for debug.
- `ui_init()` (`src/ui/ui.c`): initializes screens and loads the main screen.
- `ui_screen_start()` (`src/ui_screen.c`): initializes the clock at 00:00:00 and updates the time label every second (24h or 12h with am/pm).
- Weather service (`src/weather_service.cpp`): fetches current data at startup and then every `WEATHER_REFRESH_MINUTES`, updates UI text, and loads the icon from `icon_150.bin` using the embedded index. Forecast temperatures are formatted as `min/max°unit`.
- Weather icons (`src/weather_icons.c`): global `code/variant/bin` cache to avoid redundant decodes across targets.
- Boot progress (`src/boot_progress.c`): updates `ui_start_bar`/`ui_start_bar_texte` and switches to `ui_meteo` when ready.
- Captive portal (`src/wifi_portal.c`): AP + web UI for Wi-Fi setup, save to NVS, reboot. Scan returns up to 50 SSIDs and is refreshable in the UI; scan results are logged.
- Main function catalog: `docs/MAIN_FUNCTIONS.md`.

## Build Notes
- Platform: ESP-IDF via PlatformIO (`platformio.ini`).
- Default environment: `LVGL-320-480`.
- Weather fetch requires `esp_http_client`, `esp-tls`, `mbedtls`, `json` (declared in `src/CMakeLists.txt`).
- Weather service uses `OPENWEATHERMAP_API_KEY_3` (One Call v3) when set; otherwise it falls back to `OPENWEATHERMAP_API_KEY_2` (current weather v2.5).
- Weather mode is exclusive: full v3 (current + forecast via One Call) or full v2.5 (current + forecast), never mixed.

## Logging
- Application logs use `ESP_LOGx` with per-module tags.
- Default verbosity is controlled by PlatformIO flags:
  - `LOG_LOCAL_LEVEL` sets the compile-time maximum.
  - `APP_LOG_FILTERS=1` enables tag filtering at runtime.
  - `APP_LOG_APP_LEVEL` sets the level for application tags.
  - `APP_LOG_NOISY_LEVEL` reduces noise for system tags (wifi, gpio, sdmmc, mbedtls, cert bundle).
- SD card banners, `sdmmc_card_print_info()`, and SD directory listing are only printed when `DEBUG_LOG` is defined.
