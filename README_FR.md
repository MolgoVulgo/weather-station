# Documentation technique - Weather-Station (LVGL demo)

## Objet
Ce projet est un build PlatformIO/ESP-IDF pour la carte JC3248W535EN (ecran 320x480). Le code demarre LVGL 8.3 avec un pilote LCD AXS15231B en QSPI et un controle tactile associe.

## Structure des sources
- `src/weatherStation.c`: point d'entree `app_main()` et sequence de demarrage LVGL + init SD.
- `src/esp_bsp.c` / `src/esp_bsp.h`: BSP carte, init bus QSPI/I2C, LCD, tactile, backlight, sync tear.
- `src/lv_port.c` / `src/lv_port.h`: port LVGL (task, tick, buffers, flush, rotation, input).
- `src/display.h`: API simple pour LCD sans LVGL.
- `src/esp_lcd_axs15231b.c` / `src/esp_lcd_axs15231b.h`: pilote bas niveau AXS15231B.
- `src/esp_lcd_touch.c` / `src/esp_lcd_touch.h`: abstraction tactile ESP LCD.
- `src/lv_conf.h`: configuration LVGL compile-time.
- `src/ui/`: export EEZ Studio (UI, assets, screens). A ne pas modifier a la main.

## UI: EEZ Studio
- `ui/` (EEZ Studio): genere une structure `objects` (ex: `objects.ui_screen_label_time`) et un pipeline d'ecrans EEZ (`loadScreen`, `tick_screen`).
- Interdiction de modifier le contenu de `src/ui/` (fichiers generes).

## Variables UI (EEZ)
- Les variables exposees dans `src/ui/vars.h` sont implementees dans `src/vars.c` via `get_var_` / `set_var_`.
- L'UI met a jour les labels dans `src/ui/screens.c` a chaque `tick_screen()` en lisant ces getters.

## Flux de demarrage
1. `app_main()` appelle `setup()` dans `src/weatherStation.c`. ( squelette par défaut)
2. Logs HW (chip, flash, heap, PSRAM).
3. Creation de `bsp_display_cfg_t` (taille buffer, rotation).
4. `bsp_display_start_with_config()`:
   - `lvgl_port_init()` cree le timer LVGL + task LVGL.
   - init backlight PWM.
   - init LCD QSPI + panneau AXS15231B.
   - init tactile I2C et enregistrement LVGL input.
5. Init Wi-Fi (STA) avec credentials NVS (valeurs par defaut via `include/secrets.h`).
6. Init NVS + synchro NTP (pool configurable, offset en secondes).
7. Montage SD (/sdcard) et listing des fichiers.
8. `ui_init()` cree l'interface LVGL (EEZ Studio).
9. `ui_screen_start()` demarre l'horloge locale (timer LVGL).

## Pipeline d'affichage
- `bsp_display_new()` configure le bus QSPI et le panneau AXS15231B.
- La sequence d'init panneau est dans `lcd_init_cmds[]` (`src/esp_bsp.c`).
- `lvgl_port_add_disp()` alloue les buffers et enregistre le driver LVGL.
- `lvgl_port_flush_callback()` transfere les pixels vers le panneau, avec rotation logicielle et buffer de transport.
- La synchro tear (optionnelle) utilise une interruption GPIO et des semaphores (voir `bsp_display_sync_task`).

## Tactile
- I2C init via `bsp_i2c_init()`.
- `bsp_touch_new()` cree le handle tactile AXS15231B.
- Les coordonnees sont corrigees selon la rotation dans `bsp_touch_process_points_cb()`.

## Configuration clef
- Rotation: `LVGL_PORT_ROTATION_DEGREE` dans `src/weatherStation.c`.
- LVGL: `src/lv_conf.h`, injecte via `LV_CONF_PATH` dans `platformio.ini`.
- Taille buffer LVGL: `bsp_display_cfg_t.buffer_size` dans `src/weatherStation.c`.
- Pins QSPI/I2C: `src/esp_bsp.h`.
- NTP: pool et décalage (secondes) en NVS, namespace `time_cfg` (`src/time_sync.c`).
- Wi-Fi: SSID + mot de passe en NVS, namespace `wifi_cfg` (`src/wifi_manager.c`).

## Points d'extension
- Utiliser `bsp_display_lock()` / `bsp_display_unlock()` pour proteger les appels LVGL depuis d'autres tasks.
- Ajuster `buffer_size`, `trans_size` et la rotation pour optimiser performances/memoire.
- Ajouter capteurs/metriques dans `setup()` avant ou apres le demarrage LVGL.

### Utiliser EEZ Studio (`ui/`)
1. Garder `#include "ui_backend.h"` dans `src/weatherStation.c` et `src/ui_screen.c`.
2. Dans `platformio.ini`, activer `-D UI_BACKEND_EEZ=1`.
3. Verifier `src/CMakeLists.txt`:
   - `INCLUDE_DIRS` contient `src/ui`

## Mettre en place une UI depuis les sources generees par l'IDE

### EEZ Studio -> `src/ui/`
1. Dans EEZ Studio, exporter en LVGL/C.
2. Copier les fichiers generes dans `src/ui/`.
3. Verifier que la structure `objects` expose les bons IDs (ex: `objects.ui_screen_label_time`).
4. Adapter le code applicatif aux noms d'objets EEZ si besoin.
5. Compiler et verifier.

## Fonctions clef
- `sdcard_mount()` (`src/weatherStation.c`): monte la carte SD en SDMMC 1-bit sur `/sdcard` (CMD=GPIO11, CLK=GPIO12, D0=GPIO13).
- `sdcard_list_dir()` (`src/weatherStation.c`): logge le contenu de la SD pour debug.
- `ui_init()` (`src/ui/ui.c`): initialise les ecrans et charge l'ecran principal.
- `ui_screen_start()` (`src/ui_screen.c`): initialise l'horloge a 00:00:00 et met a jour le label d'heure chaque seconde.

## Notes build
- Plateforme: ESP-IDF via PlatformIO (`platformio.ini`).
- Environnement par defaut: `LVGL-320-480`.
