# Documentation technique - Weather-Station (LVGL demo)

## Objet
Ce projet est un build PlatformIO/ESP-IDF pour la carte JC3248W535EN (ecran 320x480). Le code demarre LVGL 8.3 avec un pilote LCD AXS15231B en QSPI et un controle tactile associe.

## Etat actuel (UI)
- Aucun changement fonctionnel en cours cote code; dernier diff git ne touche que l'etat UI EEZ (`eez/weather-station.eez-project-ui-state`).

## Structure des sources
- `src/weatherStation.c`: point d'entree `app_main()` et sequence de demarrage LVGL + init SD.
- `src/esp_bsp.c` / `src/esp_bsp.h`: BSP carte, init bus QSPI/I2C, LCD, tactile, backlight, sync tear.
- `src/lv_port.c` / `src/lv_port.h`: port LVGL (task, tick, buffers, flush, rotation, input).
- `src/display.h`: API simple pour LCD sans LVGL.
- `src/esp_lcd_axs15231b.c` / `src/esp_lcd_axs15231b.h`: pilote bas niveau AXS15231B.
- `src/esp_lcd_touch.c` / `src/esp_lcd_touch.h`: abstraction tactile ESP LCD.
- `src/lv_conf.h`: configuration LVGL compile-time.
- `src/ui/`: export EEZ Studio (UI, assets, screens). A ne pas modifier a la main.
- `libraries/weather/`: fetcher meteo ESP-IDF et mapping icones OWM (index integre).
- `src/hourly_strip.cpp` / `src/hourly_strip.h`: gestion du strip horaires (7 icones, animation et buffer local).

## UI: EEZ Studio
- `ui/` (EEZ Studio): genere une structure `objects` (ex: `objects.ui_screen_label_time`) et un pipeline d'ecrans EEZ (`loadScreen`, `tick_screen`).
- Interdiction de modifier le contenu de `src/ui/` (fichiers generes).

## Variables UI (EEZ)
- Les variables exposees dans `src/ui/vars.h` doivent etre implementees dans `src/vars.c` via `get_var_` / `set_var_` (generer/mettre a jour `src/vars.c` a chaque modification de `src/ui/vars.h`).
- L'UI met a jour les labels dans `src/ui/screens.c` a chaque `tick_screen()` en lisant ces getters.

## UI: meteo details (hourly strip)
- Le conteneur `hourly_strip` affiche 7 icones (index 2 = "now", 3..6 = futures +1..+4, 0..1 = historique -2..-1).
- Les donnees horaires proviennent du One Call v3 (tableau `hourly`, 12 entrees chargees).
- Le shift s'effectue a chaque changement d'heure si l'ecran `ui_meteo_details` est actif (animation 300ms).
- L'animation recale le X de base du conteneur au demarrage pour eviter le decalage vers la gauche au premier move.

## Internationalisation (i18n)
- Les traductions sont definies dans `src/i18n.c` et resolues via la macro `_()` de `src/lv_i18n.h`.
- La langue selectionnee est stockee en NVS (`lang_cfg`) via `src/lanague.c` et synchronisee avec le dropdown `ui_setting_laguage`.
- Les jours de previsions utilisent `app_i18n_weekday_short()` et les requetes OpenWeatherMap utilisent `lanague_get_weather_code()` pour `lang=`.

## Flux de demarrage
1. `app_main()` appelle `setup()` dans `src/weatherStation.c`. ( squelette par défaut)
2. Logs HW (chip, flash, heap, PSRAM).
3. Creation de `bsp_display_cfg_t` (taille buffer, rotation).
4. `bsp_display_start_with_config()`:
   - `lvgl_port_init()` cree le timer LVGL + task LVGL.
   - init backlight PWM.
   - init LCD QSPI + panneau AXS15231B.
   - init tactile I2C et enregistrement LVGL input.
5. `ui_init()` cree l'interface LVGL et affiche `ui_start` avec la progression.
6. Init Wi-Fi (STA) avec credentials NVS (valeurs par defaut via `include/secrets.h`), attente IP.
   - Si aucun credential ou echec repete, demarrer le portail captif AP `StationMeteo` (mot de passe `stationmeteo`) et afficher `ui_wifi`.
   - Le scan du portail logge la liste des SSID (avec RSSI) et l'UI propose un bouton de rafraichissement.
7. Synchro NTP (3 essais); log erreur si echec.
8. Montage SPIFFS + driver LVGL FS.
9. Montage SD (/sdcard) et listing des fichiers.
10. `ui_screen_start()` demarre l'horloge locale (timer LVGL).
11. Service meteo (courant + forecast) puis bascule vers `ui_meteo`.

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
- Format heure: `hour_format` dans NVS `time_cfg` (24h par defaut, 12h affiche `hh:mm am/pm`).
- Unite temperature: `temp_unit` dans NVS `weather_cfg` (0=°C, 1=°F).
- Wi-Fi: SSID + mot de passe en NVS, namespace `wifi_cfg` (`src/wifi_manager.c`).
- Reset Wi-Fi: mettre `WIFI_RESET_NVS=1` dans `include/secrets.h` pour effacer les credentials.

## Points d'extension
- Utiliser `bsp_display_lock()` / `bsp_display_unlock()` pour proteger les appels LVGL depuis d'autres tasks.
- Ajuster `buffer_size`, `trans_size` et la rotation pour optimiser performances/memoire.
- Ajouter capteurs/metriques dans `setup()` avant ou apres le demarrage LVGL.

### Utiliser EEZ Studio (`ui/`)
1. Garder `#include "ui_backend.h"` dans `src/weatherStation.c` et `src/ui_screen.c`.
2. Le backend UI est fixe sur EEZ (pas de flag de selection).
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
- `ui_screen_start()` (`src/ui_screen.c`): initialise l'horloge a 00:00:00 et met a jour le label d'heure chaque seconde (24h ou 12h avec am/pm).
- Service meteo (`src/weather_service.cpp`): recupere les donnees au demarrage puis toutes les `WEATHER_REFRESH_MINUTES`, met a jour les textes UI et charge l'icone depuis `icon_150.bin` via l'index integre. Les previsions sont formatees `min/max°unite`.
- Icônes meteo (`src/weather_icons.c`): cache global `code/variant/bin` pour eviter les decodages redondants entre cibles.
- Demarrage (`src/boot_progress.c`): met a jour `ui_start_bar`/`ui_start_bar_texte` et bascule vers `ui_meteo` une fois pret.
- Portail captif (`src/wifi_portal.c`): AP + UI web de configuration, sauvegarde NVS, reboot. Le scan retourne jusqu'a 50 SSID et est relancable via le bouton; les resultats sont logges.
- Recueil des fonctions principales: `docs/MAIN_FUNCTIONS.md`.

## Notes build
- Plateforme: ESP-IDF via PlatformIO (`platformio.ini`).
- Environnement par defaut: `LVGL-320-480`.
- La meteo requiert `esp_http_client`, `esp-tls`, `mbedtls`, `json` (declares dans `src/CMakeLists.txt`).
- Le service meteo utilise `OPENWEATHERMAP_API_KEY_3` (One Call v3) si renseigne, sinon `OPENWEATHERMAP_API_KEY_2` (meteo courante v2.5).
- Le mode meteo est exclusif: soit full v3 (courant + previsions via One Call), soit full v2.5, jamais hybride.

## Logs
- Les logs applicatifs utilisent `ESP_LOGx` avec un tag par module.
- La verbosite par defaut est pilotee via PlatformIO :
  - `LOG_LOCAL_LEVEL` fixe le niveau max au compile-time.
  - `APP_LOG_FILTERS=1` active le filtrage par tag au runtime.
  - `APP_LOG_APP_LEVEL` definit le niveau des tags applicatifs.
  - `APP_LOG_NOISY_LEVEL` reduit le bruit des tags systeme (wifi, gpio, sdmmc, mbedtls, bundle cert).
- Les bannieres SD, `sdmmc_card_print_info()` et le listing des fichiers SD ne s'affichent que si `DEBUG_LOG` est defini.
