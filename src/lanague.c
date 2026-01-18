#include "lanague.h"

#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "Lang";

#define LANG_NVS_NAMESPACE "lang_cfg"
#define LANG_NVS_KEY_ID "lang"

static const lanague_table_t lang_table_fr = {
    .day_names = {
        "Dimanche",
        "Lundi",
        "Mardi",
        "Mercredi",
        "Jeudi",
        "Vendredi",
        "Samedi",
    },
    .month_names = {
        "Janvier",
        "Fevrier",
        "Mars",
        "Avril",
        "Mai",
        "Juin",
        "Juillet",
        "Aout",
        "Septembre",
        "Octobre",
        "Novembre",
        "Decembre",
    },
};

static const lanague_table_t lang_table_en = {
    .day_names = {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday",
    },
    .month_names = {
        "January",
        "February",
        "March",
        "April",
        "May",
        "June",
        "July",
        "August",
        "September",
        "October",
        "November",
        "December",
    },
};

static const lanague_table_t lang_table_de = {
    .day_names = {
        "Sonntag",
        "Montag",
        "Dienstag",
        "Mittwoch",
        "Donnerstag",
        "Freitag",
        "Samstag",
    },
    .month_names = {
        "Januar",
        "Februar",
        "Maerz",
        "April",
        "Mai",
        "Juni",
        "Juli",
        "August",
        "September",
        "Oktober",
        "November",
        "Dezember",
    },
};

static lanague_id_t current_lang = LANAGUE_FR;

static const lanague_table_t *lanague_table_for(lanague_id_t lang)
{
    switch (lang) {
    case LANAGUE_EN:
        return &lang_table_en;
    case LANAGUE_DE:
        return &lang_table_de;
    case LANAGUE_FR:
    default:
        return &lang_table_fr;
    }
}

esp_err_t lanague_init(void)
{
    nvs_handle_t nvs = 0;
    esp_err_t ret = nvs_open(LANG_NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed: %s", esp_err_to_name(ret));
        return ret;
    }

    int32_t lang_value = (int32_t)LANAGUE_FR;
    ret = nvs_get_i32(nvs, LANG_NVS_KEY_ID, &lang_value);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        current_lang = LANAGUE_FR;
        ESP_LOGI(TAG, "NVS langue absente, defaut FR");
        ret = nvs_set_i32(nvs, LANG_NVS_KEY_ID, (int32_t)current_lang);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "NVS set langue failed: %s", esp_err_to_name(ret));
            nvs_close(nvs);
            return ret;
        }
        ret = nvs_commit(nvs);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "NVS commit failed: %s", esp_err_to_name(ret));
            nvs_close(nvs);
            return ret;
        }
    } else if (ret == ESP_OK) {
        if (lang_value == (int32_t)LANAGUE_EN) {
            current_lang = LANAGUE_EN;
        } else if (lang_value == (int32_t)LANAGUE_DE) {
            current_lang = LANAGUE_DE;
        } else {
            current_lang = LANAGUE_FR;
        }
    } else {
        ESP_LOGW(TAG, "Lecture NVS langue echouee: %s", esp_err_to_name(ret));
        current_lang = LANAGUE_FR;
    }

    nvs_close(nvs);
    return ret;
}

lanague_id_t lanague_get_current(void)
{
    return current_lang;
}

const lanague_table_t *lanague_get_table(lanague_id_t lang)
{
    return lanague_table_for(lang);
}

esp_err_t lanague_set_current(lanague_id_t lang)
{
    if (lang == LANAGUE_EN) {
        current_lang = LANAGUE_EN;
    } else if (lang == LANAGUE_DE) {
        current_lang = LANAGUE_DE;
    } else {
        current_lang = LANAGUE_FR;
    }
    return ESP_OK;
}

const char *lanague_get_locale_name(lanague_id_t lang)
{
    switch (lang) {
    case LANAGUE_EN:
        return "en-US";
    case LANAGUE_DE:
        return "de-DE";
    case LANAGUE_FR:
    default:
        return "fr-FR";
    }
}

const char *lanague_get_weather_code(lanague_id_t lang)
{
    switch (lang) {
    case LANAGUE_EN:
        return "en";
    case LANAGUE_DE:
        return "de";
    case LANAGUE_FR:
    default:
        return "fr";
    }
}

esp_err_t lanague_save_current(void)
{
    nvs_handle_t nvs = 0;
    esp_err_t ret = nvs_open(LANG_NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_set_i32(nvs, LANG_NVS_KEY_ID, (int32_t)current_lang);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS set langue failed: %s", esp_err_to_name(ret));
        nvs_close(nvs);
        return ret;
    }

    ret = nvs_commit(nvs);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS commit failed: %s", esp_err_to_name(ret));
    }
    nvs_close(nvs);
    return ret;
}
