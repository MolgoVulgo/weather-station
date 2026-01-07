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

static lanague_id_t current_lang = LANAGUE_FR;

static const lanague_table_t *lanague_table_for(lanague_id_t lang)
{
    return (lang == LANAGUE_EN) ? &lang_table_en : &lang_table_fr;
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
        nvs_set_i32(nvs, LANG_NVS_KEY_ID, (int32_t)current_lang);
        nvs_commit(nvs);
    } else if (ret == ESP_OK) {
        current_lang = (lang_value == (int32_t)LANAGUE_EN) ? LANAGUE_EN : LANAGUE_FR;
    } else {
        ESP_LOGW(TAG, "Lecture NVS langue echouee: %s", esp_err_to_name(ret));
        current_lang = LANAGUE_FR;
    }

    nvs_close(nvs);
    return ESP_OK;
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
    current_lang = (lang == LANAGUE_EN) ? LANAGUE_EN : LANAGUE_FR;
    return ESP_OK;
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
