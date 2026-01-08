# Technical Documentation - Weather-Station (LVGL demo)

## Purpose
This project is a PlatformIO/ESP-IDF build for the JC3248W535EN board (320x480 display). The code starts LVGL 8.3 with an AXS15231B LCD driver over QSPI and an associated touch controller.

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

## UI: EEZ Studio
- `ui/` (EEZ Studio): generates an `objects` structure (e.g. `objects.ui_screen_label_time`) and an EEZ screen pipeline (`loadScreen`, `tick_screen`).
- Do not modify `src/ui/` contents (generated files).

## UI Variables (EEZ)
- Variables declared in `src/ui/vars.h` must be implemented in `src/vars.c` via `get_var_` / `set_var_` (generate/update `src/vars.c` whenever `src/ui/vars.h` changes).
- The UI updates labels in `src/ui/screens.c` on each `tick_screen()` by reading these getters.

## Boot Flow
1. `app_main()` calls `setup()` in `src/weatherStation.c`. (default skeleton)
2. HW logs (chip, flash, heap, PSRAM).
3. Create `bsp_display_cfg_t` (buffer size, rotation).
4. `bsp_display_start_with_config()`:
   - `lvgl_port_init()` creates the LVGL timer + LVGL task.
   - init backlight PWM.
   - init QSPI LCD + AXS15231B panel.
   - init I2C touch and register LVGL input.
5. Wi-Fi init (STA) with NVS credentials (defaults from `include/secrets.h`).
6. NVS init + NTP sync (configurable pool, offset in seconds).
7. SD mount (`/sdcard`) and list files.
8. `ui_init()` creates the LVGL UI (EEZ Studio).
9. `ui_screen_start()` starts the local clock (LVGL timer).

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
- Wi-Fi: SSID + password in NVS, namespace `wifi_cfg` (`src/wifi_manager.c`).

## Extension Points
- Use `bsp_display_lock()` / `bsp_display_unlock()` to protect LVGL calls from other tasks.
- Adjust `buffer_size`, `trans_size`, and rotation to optimize performance/memory.
- Add sensors/metrics in `setup()` before or after LVGL startup.

### Using EEZ Studio (`ui/`)
1. Keep `#include "ui_backend.h"` in `src/weatherStation.c` and `src/ui_screen.c`.
2. In `platformio.ini`, enable `-D UI_BACKEND_EEZ=1`.
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
- `ui_screen_start()` (`src/ui_screen.c`): initializes the clock at 00:00:00 and updates the time label every second.
- Weather service (`src/weather_service.cpp`): fetches current data at startup and then every `WEATHER_REFRESH_MINUTES`, updates UI text, and loads the icon from `icon_150.bin` using the embedded index.
- Main function catalog: `docs/MAIN_FUNCTIONS.md`.

## Build Notes
- Platform: ESP-IDF via PlatformIO (`platformio.ini`).
- Default environment: `LVGL-320-480`.
- Weather fetch requires `esp_http_client`, `esp-tls`, `mbedtls`, `json` (declared in `src/CMakeLists.txt`).
- Weather service uses `OPENWEATHERMAP_API_KEY_3` (One Call v3) when set; otherwise it falls back to `OPENWEATHERMAP_API_KEY_2` (current weather v2.5).
