#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include "esp_err.h"

typedef struct {
  uint16_t code;
  uint8_t variant;
} WeatherIconRef;

bool WeatherIconResolve(int condition_id, const char* icon_code, WeatherIconRef* out);
bool WeatherIconResolve(int condition_id, const std::string& icon_code, WeatherIconRef* out);

esp_err_t WeatherIconFindOffsetStream(FILE *fp,
                                      int condition_id,
                                      const char* icon_code,
                                      uint32_t *out_offset);

esp_err_t WeatherIconFindOffset(const uint8_t *buffer,
                                size_t buffer_len,
                                int condition_id,
                                const char* icon_code,
                                uint32_t *out_offset);
