#ifndef SVG2BIN_DECODER_H
#define SVG2BIN_DECODER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef esp_err_t (*svg2bin_draw_cb_t)(
    void *user_ctx,
    const char *name,
    uint16_t width,
    uint16_t height,
    const uint8_t *rgb565,
    size_t rgb565_len);

typedef enum {
    SVG2BIN_VARIANT_DAY = 0,
    SVG2BIN_VARIANT_NIGHT = 1,
    SVG2BIN_VARIANT_NEUTRAL = 2,
} svg2bin_variant_t;

size_t svg2bin_expected_rgb565_size(uint16_t width, uint16_t height);

esp_err_t svg2bin_decode(
    const uint8_t *buffer,
    size_t buffer_len,
    svg2bin_draw_cb_t draw_cb,
    void *user_ctx);

esp_err_t svg2bin_draw_raw(
    const uint8_t *rgb565,
    size_t rgb565_len,
    const char *name,
    uint16_t width,
    uint16_t height,
    svg2bin_draw_cb_t draw_cb,
    void *user_ctx);

esp_err_t svg2bin_decode_stream(
    FILE *fp,
    svg2bin_draw_cb_t draw_cb,
    void *user_ctx);

esp_err_t svg2bin_decode_entry_at_offset(
    FILE *fp,
    uint32_t offset,
    svg2bin_draw_cb_t draw_cb,
    void *user_ctx);

esp_err_t svg2bin_find_entry_offset(
    const uint8_t *buffer,
    size_t buffer_len,
    uint16_t code,
    uint8_t variant,
    uint32_t *out_offset);

esp_err_t svg2bin_find_entry_offset_stream(
    FILE *fp,
    uint16_t code,
    uint8_t variant,
    uint32_t *out_offset);

#ifdef __cplusplus
}
#endif

#endif
