#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

typedef enum {
    LANAGUE_FR = 0,
    LANAGUE_EN = 1,
} lanague_id_t;

typedef struct {
    const char *day_names[7];
    const char *month_names[12];
} lanague_table_t;

esp_err_t lanague_init(void);
lanague_id_t lanague_get_current(void);
const lanague_table_t *lanague_get_table(lanague_id_t lang);

/* Pour future configuration Web/écran. */
esp_err_t lanague_set_current(lanague_id_t lang);
esp_err_t lanague_save_current(void);
