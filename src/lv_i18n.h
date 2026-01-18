#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *key;
    const char *value;
} lv_i18n_phrase_t;

typedef struct {
    const char *locale_name;
    const lv_i18n_phrase_t *singulars;
} lv_i18n_lang_t;

void lv_i18n_init(const lv_i18n_lang_t *langs, uint16_t lang_count);
bool lv_i18n_set_locale(const char *locale);
const char *lv_i18n_get_text(const char *key);

#define _(text) lv_i18n_get_text(text)

#ifdef __cplusplus
}
#endif
