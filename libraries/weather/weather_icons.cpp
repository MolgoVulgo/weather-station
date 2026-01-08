#include "weather_icons.h"

#include <cstring>
#include "svg2bin_decoder.h"

static uint8_t icon_variant_from_code(const char *icon_code) {
  if (!icon_code || !*icon_code) {
    return SVG2BIN_VARIANT_NEUTRAL;
  }
  size_t len = strlen(icon_code);
  if (len == 0) {
    return SVG2BIN_VARIANT_NEUTRAL;
  }
  char suffix = icon_code[len - 1];
  if (suffix == 'd') {
    return SVG2BIN_VARIANT_DAY;
  }
  if (suffix == 'n') {
    return SVG2BIN_VARIANT_NIGHT;
  }
  return SVG2BIN_VARIANT_NEUTRAL;
}

bool WeatherIconResolve(int condition_id, const char* icon_code, WeatherIconRef* out) {
  if (!out || condition_id <= 0) {
    return false;
  }
  out->code = static_cast<uint16_t>(condition_id);
  out->variant = icon_variant_from_code(icon_code);
  return true;
}

bool WeatherIconResolve(int condition_id, const std::string& icon_code, WeatherIconRef* out) {
  return WeatherIconResolve(condition_id, icon_code.c_str(), out);
}

esp_err_t WeatherIconFindOffsetStream(FILE *fp,
                                      int condition_id,
                                      const char* icon_code,
                                      uint32_t *out_offset) {
  WeatherIconRef ref;
  if (!WeatherIconResolve(condition_id, icon_code, &ref)) {
    return ESP_ERR_INVALID_ARG;
  }
  esp_err_t err = svg2bin_find_entry_offset_stream(fp, ref.code, ref.variant, out_offset);
  if (err == ESP_ERR_NOT_FOUND && ref.variant != SVG2BIN_VARIANT_NEUTRAL) {
    err = svg2bin_find_entry_offset_stream(fp, ref.code, SVG2BIN_VARIANT_NEUTRAL, out_offset);
  }
  return err;
}

esp_err_t WeatherIconFindOffset(const uint8_t *buffer,
                                size_t buffer_len,
                                int condition_id,
                                const char* icon_code,
                                uint32_t *out_offset) {
  WeatherIconRef ref;
  if (!WeatherIconResolve(condition_id, icon_code, &ref)) {
    return ESP_ERR_INVALID_ARG;
  }
  esp_err_t err = svg2bin_find_entry_offset(buffer, buffer_len, ref.code, ref.variant, out_offset);
  if (err == ESP_ERR_NOT_FOUND && ref.variant != SVG2BIN_VARIANT_NEUTRAL) {
    err = svg2bin_find_entry_offset(buffer, buffer_len, ref.code, SVG2BIN_VARIANT_NEUTRAL, out_offset);
  }
  return err;
}
