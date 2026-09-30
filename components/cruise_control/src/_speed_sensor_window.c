// #include "speed_sensor.h"
// #include <stdint.h>
// #include "config.h"
// #include "gpio_hall.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/semphr.h"
// #include "esp_log.h"
// #include "oled.h"
// #include "esp_timer.h"

// static volatile uint32_t pulse_count = 0; 
// static volatile int64_t last_pulse_time = 0;

// static  float pulse_count_per_second = 0;
// static float last_speed_filtered = 0.0f;

// static const int64_t MIN_PULSE_INTERVAL_US =
//     (int64_t)(1000000.0f /
//     (((SPEED_SENSOR_MAX_SPEED_KMH / 3.6f) /
//     SPEED_SENSOR_WHEEL_CIRCUMFERENCE_METERS) *
//     SPEED_SENSOR_PULSES_PER_WHEEL_TURN));

// static const float MAX_PULSES_PER_SECOND =
//     ((SPEED_SENSOR_MAX_SPEED_KMH / 3.6f) /
//     SPEED_SENSOR_WHEEL_CIRCUMFERENCE_METERS)
//     *
//     SPEED_SENSOR_PULSES_PER_WHEEL_TURN;

// static const float SPEED_SENSOR_WINDOW_CORRECTION_FACTOR = 1000.0f / SPEED_SENSOR_MEASUREMENT_WINDOW_MS;

// TaskHandle_t show_speed_task_handle;

// portMUX_TYPE pulse_count_mux = portMUX_INITIALIZER_UNLOCKED;

// speed_t get_current_speed(){
//     speed_t speed;

//     portENTER_CRITICAL(&pulse_count_mux);

//     float turns_per_second = pulse_count_per_second / SPEED_SENSOR_PULSES_PER_WHEEL_TURN;

//     portEXIT_CRITICAL(&pulse_count_mux);

//     float meters_per_second = turns_per_second * SPEED_SENSOR_WHEEL_CIRCUMFERENCE_METERS;
//     float km_h = meters_per_second * 3.6f; // m/s -> km/h (3600 / 1000 = 3.6)

//     speed.raw = km_h;
//     km_h *= SPEED_SENSOR_CALIBRATION_FACTOR;


//     if(last_speed_filtered == 0.0f){
//         last_speed_filtered = km_h;
//         speed.filtered = last_speed_filtered;

//         return speed;
//     }
    
//     last_speed_filtered = last_speed_filtered + 0.15f * (km_h - last_speed_filtered);
//     speed.filtered = last_speed_filtered;

//     return speed;
// }

// void show_speed_task(void *pvParameters){
//     while(1){
//         ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

//         char buffer_filtered[7];
//         char buffer_raw[7];

//         speed_t current_speed = get_current_speed();
        
//         snprintf(buffer_filtered, sizeof(buffer_filtered), "%-3.2f", current_speed.filtered);
//         snprintf(buffer_raw, sizeof(buffer_raw), "%-3.2f", current_speed.raw);

//         // oled_print(buffer);
//         oled_print_two_lines(buffer_filtered, buffer_raw);
//     }
// }

// void IRAM_ATTR speed_sensor_isr(void *args){
//     int64_t now = esp_timer_get_time();

//     portENTER_CRITICAL_ISR(&pulse_count_mux);

//     if((now - last_pulse_time) >= MIN_PULSE_INTERVAL_US){

//         pulse_count++;
//         last_pulse_time = now;

//     }
    
//     portEXIT_CRITICAL_ISR(&pulse_count_mux);
// }

// void update_pulse_counters_task(void *pvParameters){
//     while(1){
//         vTaskDelay(pdMS_TO_TICKS(SPEED_SENSOR_MEASUREMENT_WINDOW_MS));
        
//         portENTER_CRITICAL(&pulse_count_mux);
        
//         float new_pulse_count_per_second = pulse_count * SPEED_SENSOR_WINDOW_CORRECTION_FACTOR;

//         if(new_pulse_count_per_second <= MAX_PULSES_PER_SECOND){
//             pulse_count_per_second = new_pulse_count_per_second;
//         }

//         pulse_count = 0;
        
//         portEXIT_CRITICAL(&pulse_count_mux);

//         xTaskNotifyGive(show_speed_task_handle);
//     }
// }

// void speed_sensor_init(void){
//     gpio_init(SPEED_SENSOR_GPIO, GPIO_MODE_INPUT, GPIO_PULLUP_DISABLE, GPIO_PULLDOWN_DISABLE, GPIO_INTR_POSEDGE);

//     xTaskCreate(update_pulse_counters_task, "update_pulse_counters_task", 2048, NULL, 4, NULL);
//     xTaskCreate(show_speed_task, "show_speed_task", 2048, NULL, 3, &show_speed_task_handle);

//     gpio_isr_handler_add(SPEED_SENSOR_GPIO, speed_sensor_isr, NULL);

//     ESP_LOGI(__func__, "Speed sensor initialized successfully.");
// }