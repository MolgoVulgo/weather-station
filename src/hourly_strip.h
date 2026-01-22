#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

void hourly_strip_tick(const struct tm *timeinfo, bool details_active);
void hourly_strip_detail_chart_ensure(void);

#ifdef __cplusplus
}

#include "weather_fetcher.h"

void hourly_strip_update(const CurrentWeatherData *current,
                         const HourlyEntry *hourly,
                         size_t hourly_count,
                         time_t now_ts);
#endif
