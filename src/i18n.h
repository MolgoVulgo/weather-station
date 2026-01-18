#pragma once

#include "lanague.h"
#include "lv_i18n.h"

#ifdef __cplusplus
extern "C" {
#endif

void app_i18n_init(lanague_id_t lang);
void app_i18n_set_language(lanague_id_t lang);
const char *app_i18n_weekday_short(int weekday);

#ifdef __cplusplus
}
#endif
