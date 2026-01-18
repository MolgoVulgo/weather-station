#include "i18n.h"

#include <stddef.h>
#include "lanague.h"

static const lv_i18n_phrase_t kLangFr[] = {
    {"Station Météo", "Station Météo"},
    {"Format de l'heure :", "Format de l'heure :"},
    {"Décalage horaire :", "Décalage horaire :"},
    {"Unité :", "Unité :"},
    {"Unité : °C", "Unité : °C"},
    {"Configuration Wifi", "Configuration Wifi"},
    {"Avancer", "Avancer"},
    {"Francais\nanglais\nAllemand", "Francais\nanglais\nAllemand"},
    {"Demarrage...", "Demarrage..."},
    {"WiFi...", "WiFi..."},
    {"WiFi OK", "WiFi OK"},
    {"WiFi ERR", "WiFi ERR"},
    {"AP MODE", "MODE AP"},
    {"API KEY", "CLE API"},
    {"NTP OK", "NTP OK"},
    {"NTP ERR", "NTP ERREUR"},
    {"Meteo", "Meteo"},
    {"Forecast", "Previsions"},
    {"DIM", "DIM"},
    {"LUN", "LUN"},
    {"MAR", "MAR"},
    {"MER", "MER"},
    {"JEU", "JEU"},
    {"VEN", "VEN"},
    {"SAM", "SAM"},
    {NULL, NULL},
};

static const lv_i18n_phrase_t kLangEn[] = {
    {"Station Météo", "Weather Station"},
    {"Format de l'heure :", "Time format:"},
    {"Décalage horaire :", "Time offset:"},
    {"Unité :", "Unit:"},
    {"Unité : °C", "Unit: °C"},
    {"Configuration Wifi", "WiFi setup"},
    {"Avancer", "Next"},
    {"Francais\nanglais\nAllemand", "French\nEnglish\nGerman"},
    {"Demarrage...", "Starting..."},
    {"WiFi...", "WiFi..."},
    {"WiFi OK", "WiFi OK"},
    {"WiFi ERR", "WiFi ERR"},
    {"AP MODE", "AP MODE"},
    {"API KEY", "API KEY"},
    {"NTP OK", "NTP OK"},
    {"NTP ERR", "NTP ERR"},
    {"Meteo", "Weather"},
    {"Forecast", "Forecast"},
    {"DIM", "SUN"},
    {"LUN", "MON"},
    {"MAR", "TUE"},
    {"MER", "WED"},
    {"JEU", "THU"},
    {"VEN", "FRI"},
    {"SAM", "SAT"},
    {NULL, NULL},
};

static const lv_i18n_phrase_t kLangDe[] = {
    {"Station Météo", "Wetterstation"},
    {"Format de l'heure :", "Uhrformat:"},
    {"Décalage horaire :", "Zeitzone:"},
    {"Unité :", "Einheit:"},
    {"Unité : °C", "Einheit: °C"},
    {"Configuration Wifi", "WLAN-Konfiguration"},
    {"Avancer", "Weiter"},
    {"Francais\nanglais\nAllemand", "Franzoesisch\nEnglisch\nDeutsch"},
    {"Demarrage...", "Startet..."},
    {"WiFi...", "WLAN..."},
    {"WiFi OK", "WLAN OK"},
    {"WiFi ERR", "WLAN FEHLER"},
    {"AP MODE", "AP-MODUS"},
    {"API KEY", "API-SCHLUESSEL"},
    {"NTP OK", "NTP OK"},
    {"NTP ERR", "NTP FEHLER"},
    {"Meteo", "Wetter"},
    {"Forecast", "Vorhersage"},
    {"DIM", "SO"},
    {"LUN", "MO"},
    {"MAR", "DI"},
    {"MER", "MI"},
    {"JEU", "DO"},
    {"VEN", "FR"},
    {"SAM", "SA"},
    {NULL, NULL},
};

static const lv_i18n_lang_t kLangs[] = {
    {.locale_name = "fr-FR", .singulars = kLangFr},
    {.locale_name = "en-US", .singulars = kLangEn},
    {.locale_name = "de-DE", .singulars = kLangDe},
};

static const char *kWeekdayKeys[] = {"DIM", "LUN", "MAR", "MER", "JEU", "VEN", "SAM"};

void app_i18n_init(lanague_id_t lang)
{
    lv_i18n_init(kLangs, (uint16_t)(sizeof(kLangs) / sizeof(kLangs[0])));
    app_i18n_set_language(lang);
}

void app_i18n_set_language(lanague_id_t lang)
{
    const char *locale = lanague_get_locale_name(lang);
    if (!locale) {
        locale = "fr-FR";
    }
    lv_i18n_set_locale(locale);
}

const char *app_i18n_weekday_short(int weekday)
{
    if (weekday < 0 || weekday >= (int)(sizeof(kWeekdayKeys) / sizeof(kWeekdayKeys[0]))) {
        return "";
    }
    return _(kWeekdayKeys[weekday]);
}
