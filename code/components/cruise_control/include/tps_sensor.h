#pragma once

#include "esp_adc/adc_oneshot.h"

float tps_get_throttle_position(void);
adc_oneshot_unit_handle_t tps_get_adc_handle(void);
void tps_sensor_init(adc_oneshot_unit_handle_t adc_handle);