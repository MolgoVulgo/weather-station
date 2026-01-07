# SVG2BIN

Outil Python pour convertir des fichiers SVG en RGB565 et les emballer dans un format binaire avec en-tetes. Une petite lib C (ESP-IDF) permet de decoder ce format et d'afficher sur un LCD.

## Prerequis

- Python 3
- `cairosvg` et `pillow`

## Utilisation (Python)

Conversion d'un seul fichier (une entree dans le .bin):

```bash
python3 svg2bin.py input.svg --size 128x64 --out output.bin
```

Conversion d'un repertoire (merge automatique):

```bash
python3 svg2bin.py assets/ --size 128x64 --out merged.bin
```

Conversion d'un repertoire avec un .bin par SVG en plus du merge:

```bash
python3 svg2bin.py assets/ --size 128x64 --out merged.bin --out-dir bins/
```

Compression optionnelle (flag present dans le format, non supportee dans le decodeur ESP-IDF):

```bash
python3 svg2bin.py assets/ --size 128x64 --out merged.bin --out-dir bins/ --compress
```

## Format binaire (.bin)

Le fichier merge commence par un en-tete d'index, puis les entrees images.

En-tete:
- `char[4] magic` = "S2BI"
- `uint16 version` = 1
- `uint16 index_count`

Index (repete `index_count`):
- `uint16 code` (OWM)
- `uint8 variant` (0=jour, 1=nuit, 2=neutre)
- `uint8 reserved` (0)
- `uint32 offset` (offset absolu vers l'entree image)

Entree image (a l'offset):
- `uint16 name_len`
- `name_len` bytes: nom (UTF-8)
- `uint16 width`
- `uint16 height`
- `uint32 data_len`
- `uint8 compressed` (0 ou 1)
- `data_len` bytes: payload RGB565 (brut). Le flag `compressed` est reserve.

Notes:
- `width` et `height` sont en pixels.
- `data_len` est la taille du payload (brut ou compresse).
- RGB565 est en little-endian, 2 bytes par pixel, ordre R(5)-G(6)-B(5).

## Decodeur (ESP-IDF)

Ajouter `svg2bin_decoder.c` et `svg2bin_decoder.h` dans votre composant. Le decodeur lit les entrees et appelle un callback pour transmettre les pixels RGB565 au LCD.

### Fonctions exposees

`size_t svg2bin_expected_rgb565_size(uint16_t width, uint16_t height)`

- Retourne la taille attendue (en bytes) pour une image RGB565 de `width` x `height`.
- Formule: `width * height * 2`.

`esp_err_t svg2bin_decode(const uint8_t *buffer, size_t buffer_len, svg2bin_draw_cb_t draw_cb, void *user_ctx)`

- Decode un buffer contenant une ou plusieurs entrees consecutives (format `.bin`).
- Pour chaque entree:
  - Lit l'en-tete (nom, dimensions, taille payload, flag compression).
  - Decompresse si `compressed == 1` (zlib).
  - Verifie la taille attendue.
  - Appelle `draw_cb` avec le pointeur RGB565.
- Retourne `ESP_OK` si toutes les entrees sont decodees, sinon un code d'erreur.

`esp_err_t svg2bin_decode_stream(FILE *fp, svg2bin_draw_cb_t draw_cb, void *user_ctx)`

- Decode un fichier `.bin` en streaming (pas de buffer global).
- Lit l'en-tete puis le payload entree par entree depuis `FILE *`.
- Retourne `ESP_OK` si toutes les entrees sont decodees, sinon un code d'erreur.

`esp_err_t svg2bin_decode_entry_at_offset(FILE *fp, uint32_t offset, svg2bin_draw_cb_t draw_cb, void *user_ctx)`

- Decode une seule entree a partir d'un offset (en bytes) dans le `.bin`.
- Utilise `fseek` puis lit l'entree a partir de cet offset.
- Retourne `ESP_OK` si l'entree est decodee, sinon un code d'erreur.

`esp_err_t svg2bin_draw_raw(const uint8_t *rgb565, size_t rgb565_len, const char *name, uint16_t width, uint16_t height, svg2bin_draw_cb_t draw_cb, void *user_ctx)`

- Valide que `rgb565_len` correspond a `width * height * 2`.
- Appelle directement `draw_cb`.
- Utile si vous avez deja un buffer RGB565 brut.

### Callback

`typedef esp_err_t (*svg2bin_draw_cb_t)(void *user_ctx, const char *name, uint16_t width, uint16_t height, const uint8_t *rgb565, size_t rgb565_len);`

- `name`: nom de l'entree (stem du fichier SVG).
- `rgb565`: buffer en RGB565 little-endian.
- `rgb565_len`: taille en bytes.

### Exemple minimal

```c
#include "svg2bin_decoder.h"

extern esp_err_t lcgl_draw_rgb565(const uint8_t *data, uint16_t w, uint16_t h);

static esp_err_t draw_cb(void *ctx, const char *name, uint16_t w, uint16_t h,
                         const uint8_t *rgb565, size_t len)
{
    (void)ctx;
    (void)name;
    return lcgl_draw_rgb565(rgb565, w, h);
}

void app_main(void)
{
    extern const uint8_t *buffer;
    extern size_t buffer_len;

    esp_err_t rc = svg2bin_decode(buffer, buffer_len, draw_cb, NULL);
    if (rc != ESP_OK) {
        // gestion d'erreur
    }
}
```

Dependance: zlib (`REQUIRES zlib` dans le CMakeLists.txt du composant).
