#include "svg2bin_decoder.h"

#include <stdlib.h>
#include <string.h>


static bool read_le16(const uint8_t *buf, size_t len, size_t *offset, uint16_t *out)
{
    if (*offset + 2 > len) {
        return false;
    }
    *out = (uint16_t)buf[*offset] | ((uint16_t)buf[*offset + 1] << 8);
    *offset += 2;
    return true;
}

static bool read_le32(const uint8_t *buf, size_t len, size_t *offset, uint32_t *out)
{
    if (*offset + 4 > len) {
        return false;
    }
    *out = (uint32_t)buf[*offset]
        | ((uint32_t)buf[*offset + 1] << 8)
        | ((uint32_t)buf[*offset + 2] << 16)
        | ((uint32_t)buf[*offset + 3] << 24);
    *offset += 4;
    return true;
}

static bool read_u8(const uint8_t *buf, size_t len, size_t *offset, uint8_t *out)
{
    if (*offset + 1 > len) {
        return false;
    }
    *out = buf[*offset];
    *offset += 1;
    return true;
}

static bool stream_read_exact(FILE *fp, uint8_t *buf, size_t len)
{
    return fread(buf, 1, len, fp) == len;
}

static bool stream_read_le16(FILE *fp, uint16_t *out)
{
    uint8_t tmp[2];
    if (!stream_read_exact(fp, tmp, sizeof(tmp))) {
        return false;
    }
    *out = (uint16_t)tmp[0] | ((uint16_t)tmp[1] << 8);
    return true;
}

static bool stream_read_le32(FILE *fp, uint32_t *out)
{
    uint8_t tmp[4];
    if (!stream_read_exact(fp, tmp, sizeof(tmp))) {
        return false;
    }
    *out = (uint32_t)tmp[0]
        | ((uint32_t)tmp[1] << 8)
        | ((uint32_t)tmp[2] << 16)
        | ((uint32_t)tmp[3] << 24);
    return true;
}

static bool stream_read_u8(FILE *fp, uint8_t *out)
{
    uint8_t tmp = 0;
    if (!stream_read_exact(fp, &tmp, 1)) {
        return false;
    }
    *out = tmp;
    return true;
}

static esp_err_t svg2bin_decode_entry_from_stream(
    FILE *fp,
    svg2bin_draw_cb_t draw_cb,
    void *user_ctx)
{
    uint16_t name_len = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint32_t data_len = 0;
    uint8_t compressed = 0;

    if (!stream_read_le16(fp, &name_len)) {
        return ESP_ERR_INVALID_SIZE;
    }
    if (name_len == 0) {
        return ESP_ERR_INVALID_SIZE;
    }

    char *name = (char *)malloc((size_t)name_len + 1u);
    if (!name) {
        return ESP_ERR_NO_MEM;
    }
    if (!stream_read_exact(fp, (uint8_t *)name, name_len)) {
        free(name);
        return ESP_ERR_INVALID_SIZE;
    }
    name[name_len] = '\0';

    if (!stream_read_le16(fp, &width)
        || !stream_read_le16(fp, &height)
        || !stream_read_le32(fp, &data_len)
        || !stream_read_u8(fp, &compressed)) {
        free(name);
        return ESP_ERR_INVALID_SIZE;
    }

    if (compressed) {
        free(name);
        return ESP_ERR_NOT_SUPPORTED;
    }

    size_t expected = svg2bin_expected_rgb565_size(width, height);
    if (data_len != expected) {
        free(name);
        return ESP_ERR_INVALID_SIZE;
    }

    uint8_t *payload = (uint8_t *)malloc(expected);
    if (!payload) {
        free(name);
        return ESP_ERR_NO_MEM;
    }
    if (!stream_read_exact(fp, payload, expected)) {
        free(payload);
        free(name);
        return ESP_ERR_INVALID_SIZE;
    }

    esp_err_t rc = draw_cb(user_ctx, name, width, height, payload, expected);
    free(payload);
    free(name);
    return rc;
}

size_t svg2bin_expected_rgb565_size(uint16_t width, uint16_t height)
{
    return (size_t)width * (size_t)height * 2u;
}

esp_err_t svg2bin_draw_raw(
    const uint8_t *rgb565,
    size_t rgb565_len,
    const char *name,
    uint16_t width,
    uint16_t height,
    svg2bin_draw_cb_t draw_cb,
    void *user_ctx)
{
    if (!rgb565 || !draw_cb) {
        return ESP_ERR_INVALID_ARG;
    }
    size_t expected = svg2bin_expected_rgb565_size(width, height);
    if (rgb565_len != expected) {
        return ESP_ERR_INVALID_SIZE;
    }
    return draw_cb(user_ctx, name, width, height, rgb565, rgb565_len);
}


esp_err_t svg2bin_decode(
    const uint8_t *buffer,
    size_t buffer_len,
    svg2bin_draw_cb_t draw_cb,
    void *user_ctx)
{
    if (!buffer || !draw_cb) {
        return ESP_ERR_INVALID_ARG;
    }

    size_t offset = 0;
    while (offset < buffer_len) {
        uint16_t name_len = 0;
        uint16_t width = 0;
        uint16_t height = 0;
        uint32_t data_len = 0;
        uint8_t compressed = 0;

        if (!read_le16(buffer, buffer_len, &offset, &name_len)) {
            return ESP_ERR_INVALID_SIZE;
        }
        if (offset + name_len > buffer_len) {
            return ESP_ERR_INVALID_SIZE;
        }
        char *name = (char *)malloc((size_t)name_len + 1u);
        if (!name) {
            return ESP_ERR_NO_MEM;
        }
        memcpy(name, buffer + offset, name_len);
        name[name_len] = '\0';
        offset += name_len;

        if (!read_le16(buffer, buffer_len, &offset, &width)
            || !read_le16(buffer, buffer_len, &offset, &height)
            || !read_le32(buffer, buffer_len, &offset, &data_len)
            || !read_u8(buffer, buffer_len, &offset, &compressed)) {
            free(name);
            return ESP_ERR_INVALID_SIZE;
        }

        if (offset + data_len > buffer_len) {
            free(name);
            return ESP_ERR_INVALID_SIZE;
        }

        const uint8_t *payload = buffer + offset;
        offset += data_len;

        esp_err_t rc;
        if (compressed) {
            free(name);
            return ESP_ERR_NOT_SUPPORTED;
        } else {
            size_t expected = svg2bin_expected_rgb565_size(width, height);
            if (data_len != expected) {
                free(name);
                return ESP_ERR_INVALID_SIZE;
            }
            rc = draw_cb(user_ctx, name, width, height, payload, data_len);
        }

        free(name);
        if (rc != ESP_OK) {
            return rc;
        }
    }

    return ESP_OK;
}

esp_err_t svg2bin_decode_stream(
    FILE *fp,
    svg2bin_draw_cb_t draw_cb,
    void *user_ctx)
{
    if (!fp || !draw_cb) {
        return ESP_ERR_INVALID_ARG;
    }

    while (1) {
        int c = fgetc(fp);
        if (c == EOF) {
            return ESP_OK;
        }
        ungetc(c, fp);

        esp_err_t rc = svg2bin_decode_entry_from_stream(fp, draw_cb, user_ctx);
        if (rc != ESP_OK) {
            return rc;
        }
    }
}

esp_err_t svg2bin_decode_entry_at_offset(
    FILE *fp,
    uint32_t offset,
    svg2bin_draw_cb_t draw_cb,
    void *user_ctx)
{
    if (!fp || !draw_cb) {
        return ESP_ERR_INVALID_ARG;
    }
    if (fseek(fp, (long)offset, SEEK_SET) != 0) {
        return ESP_FAIL;
    }
    return svg2bin_decode_entry_from_stream(fp, draw_cb, user_ctx);
}
