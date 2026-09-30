#include "tps_sensor.h"
#include "config.h"
#include "adc_hall.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static adc_oneshot_unit_handle_t tps_adc_handle;
static adc_cali_handle_t tps_adc_cali_handle;

float tps_get_throttle_position(){
    int raw = 0;
    int voltage = 0;

    adc_read(tps_adc_handle, ACCELERATOR_SENSOR_CHANNEL, &raw);  
    adc_get_voltage(tps_adc_cali_handle, raw, &voltage);

    float real_voltage = voltage / 1000.0f;
    float real_voltage_percent = ((real_voltage - ACCELERATOR_MINIMUM_VOLTAGE) / (ACCELERATOR_MAXIMUM_VOLTAGE - ACCELERATOR_MINIMUM_VOLTAGE)) * 100.0f;

    if(real_voltage_percent < ACCELERATOR_MINIMUM_TOLERANCE){
        real_voltage_percent = 0.0f;
    }else if(real_voltage_percent > ACCELERATOR_MAXIMUM_TOLERANCE){
        real_voltage_percent = 100.0f;
    }
    
    return real_voltage_percent;
}

adc_oneshot_unit_handle_t tps_get_adc_handle(){
    return tps_adc_handle;
}

void tps_sensor_init(adc_oneshot_unit_handle_t adc_handle){
    tps_adc_handle = adc_handle;

    adc_add(tps_adc_handle, ACCELERATOR_SENSOR_ATTEN, ACCELERATOR_SENSOR_CHANNEL);
    tps_adc_cali_handle = adc_calibration(ACCELERATOR_SENSOR_ATTEN, ACCELERATOR_SENSOR_CHANNEL);

    ESP_LOGI(__func__, "Acceleration sensor initialized successfully.");

    // Used to test
    // while(1){
    //     float current_acceleration = tps_get_throttle_position();
    //     ESP_LOGI(__func__, "Current acceleration: %.3f", current_acceleration);

    //     vTaskDelay(pdMS_TO_TICKS(200));
    // }
}