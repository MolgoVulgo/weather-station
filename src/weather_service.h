#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void weather_service_start(void);
void weather_service_request_update(void);
void weather_service_refresh_forecast_units(void);

#ifdef __cplusplus
}
#endif
