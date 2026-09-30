#pragma once

#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"

adc_oneshot_unit_handle_t adc_init(void);
void adc_add(adc_oneshot_unit_handle_t adc_handle, adc_atten_t atten, adc_channel_t channel);
adc_cali_handle_t adc_calibration(adc_atten_t atten, adc_channel_t channel);
esp_err_t adc_read(adc_oneshot_unit_handle_t adc_handle, adc_channel_t channel, int *raw);
esp_err_t adc_get_voltage(adc_cali_handle_t adc_calibration_handle, int raw, int *voltage);
esp_err_t adc_remove(adc_oneshot_unit_handle_t adc_handle, adc_cali_handle_t *adc_calibration_handle);