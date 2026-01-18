#include "lv_i18n.h"

#include <string.h>

static const lv_i18n_lang_t *s_langs;
static uint16_t s_lang_count;
static const lv_i18n_lang_t *s_current;

void lv_i18n_init(const lv_i18n_lang_t *langs, uint16_t lang_count)
{
    s_langs = langs;
    s_lang_count = lang_count;
    s_current = (langs && lang_count > 0) ? &langs[0] : NULL;
}

bool lv_i18n_set_locale(const char *locale)
{
    if (!s_langs || s_lang_count == 0 || !locale) {
        return false;
    }

    for (uint16_t i = 0; i < s_lang_count; ++i) {
        if (s_langs[i].locale_name && strcmp(s_langs[i].locale_name, locale) == 0) {
            s_current = &s_langs[i];
            return true;
        }
    }

    s_current = &s_langs[0];
    return false;
}

const char *lv_i18n_get_text(const char *key)
{
    if (!key || !s_current || !s_current->singulars) {
        return key;
    }

    for (const lv_i18n_phrase_t *phrase = s_current->singulars; phrase->key; ++phrase) {
        if (strcmp(phrase->key, key) == 0) {
            return phrase->value ? phrase->value : key;
        }
    }
    return key;
}
