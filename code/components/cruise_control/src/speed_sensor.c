#include "speed_sensor.h"
#include <stdint.h>
#include "config.h"
#include "gpio_hall.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "oled.h"
#include "esp_timer.h"

volatile int64_t last_pulse_time = 0;
volatile int64_t pulse_delta = 0;

static float last_speed_filtered = 0.0f;

static const int64_t MIN_PULSE_INTERVAL_US =
    (int64_t)(1000000.0f /
    (((SPEED_SENSOR_MAX_SPEED_KMH / 3.6f) /
    SPEED_SENSOR_WHEEL_CIRCUMFERENCE_METERS) *
    SPEED_SENSOR_PULSES_PER_WHEEL_TURN));

static const int64_t MAX_PULSE_INTERVAL_US =
    (int64_t)(1000000.0f /
    (((1.0f / 3.6f) /
    SPEED_SENSOR_WHEEL_CIRCUMFERENCE_METERS) *
    SPEED_SENSOR_PULSES_PER_WHEEL_TURN));

portMUX_TYPE speed_sensor_mux = portMUX_INITIALIZER_UNLOCKED;

float get_current_speed(){
    portENTER_CRITICAL(&speed_sensor_mux);

    int64_t delta = pulse_delta;

    portEXIT_CRITICAL(&speed_sensor_mux);

    if(delta == 0 || delta > MAX_PULSE_INTERVAL_US){
        return 0.0f;
    }

    float pulse_time_seconds = delta / 1000000.0f;
    float wheel_turn_time = pulse_time_seconds * SPEED_SENSOR_PULSES_PER_WHEEL_TURN;
    float turns_per_second = 1.0f / wheel_turn_time;
    float meters_per_second = turns_per_second * SPEED_SENSOR_WHEEL_CIRCUMFERENCE_METERS;
    float km_h = meters_per_second * 3.6f;

    km_h *= SPEED_SENSOR_CALIBRATION_FACTOR;

    if(last_speed_filtered == 0.0f){
        last_speed_filtered = km_h;
        return last_speed_filtered;
    }
    
    last_speed_filtered = last_speed_filtered + SPEED_SENSOR_FILTER_ALPHA * (km_h - last_speed_filtered);

    return last_speed_filtered;
}

void show_speed_task(void *pvParameters){
    while(1){
        char buffer[7];

        float current_speed = get_current_speed();

        snprintf(buffer, sizeof(buffer), "%-3.0f", current_speed);

        oled_print(buffer, 5);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void IRAM_ATTR speed_sensor_isr(void *args){ 
    int64_t now = esp_timer_get_time();

    portENTER_CRITICAL_ISR(&speed_sensor_mux);

    if(last_pulse_time == 0){
        last_pulse_time = now;
        portEXIT_CRITICAL_ISR(&speed_sensor_mux);
        return;
    }

    int64_t delta = now - last_pulse_time;

    if(delta >= MIN_PULSE_INTERVAL_US){
        pulse_delta = delta;
        last_pulse_time = now;
    }

    portEXIT_CRITICAL_ISR(&speed_sensor_mux);
}

void speed_sensor_init(void){
    gpio_init(SPEED_SENSOR_GPIO, GPIO_MODE_INPUT, GPIO_PULLUP_DISABLE, GPIO_PULLDOWN_DISABLE, GPIO_INTR_POSEDGE);

    xTaskCreate(show_speed_task, "show_speed_task", 2048, NULL, 3, NULL);

    gpio_isr_handler_add(SPEED_SENSOR_GPIO, speed_sensor_isr, NULL);

    ESP_LOGI(__func__, "Speed sensor initialized successfully.");
}