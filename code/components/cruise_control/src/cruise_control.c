#include "cruise_control.h"
#include "config.h"
#include "gpio_hall.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdbool.h>
#include "speed_sensor.h"
#include "tps_sensor.h"
#include "motor_control.h"
#include <stdint.h>
#include "pid.h"
#include "brake_sensor.h"
#include "clutch_sensor.h"
#include "control_button.h"
#include "led.h"
#include "esp_timer.h"
#include "oled.h"

typedef enum {
    AUTOPILOT_DISABLED,
    AUTOPILOT_ACTIVATING,
    AUTOPILOT_ACTIVE,
    AUTOPILOT_DEACTIVATING
} autopilot_state_t;

TaskHandle_t pilot_task_handle;
TaskHandle_t calibration_task_handle;
TaskHandle_t emergency_release_task_handle;

static float speed_target = 0;

static autopilot_state_t autopilot_state = AUTOPILOT_DISABLED;

TaskHandle_t get_pilot_task_handle(void){
    return pilot_task_handle;
}

TaskHandle_t get_calibration_task_handle(void){
    return calibration_task_handle;
}

TaskHandle_t get_emergency_release_task_handle(void){
    return emergency_release_task_handle;
}

void emergency_release_task(void *pvParameter){
    while(1){
        xTaskNotifyWait(0, UINT32_MAX, NULL, portMAX_DELAY);

        if(get_current_speed() != 0.0f || auto_pilot_is_active()){
            continue;
        }

        if(!control_button_confirmed_press(8000)){
            oled_show_message(OLED_MESSAGE_BUTTON_NOT_CONFIRMED);
            continue;
        }

        led_set_blink_now(true, 50);

        motor_close_throttle(255);

        oled_show_motor_emergency_releasing(true);

        while(control_button_is_pressed()){
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        while(ulTaskNotifyTake(pdTRUE, 0) > 0){
            ESP_LOGW(__func__, "Discarded spurious button trigger during hold.\n");
        }
    }
}

void calibration_task(void *pvParameter){
    while(1){
        xTaskNotifyWait(0, UINT32_MAX, NULL, portMAX_DELAY);

        if(get_current_speed() != 0.0f || auto_pilot_is_active()){
            continue;
        }

        if(!control_button_confirmed_press(3000)){
            oled_show_message(OLED_MESSAGE_BUTTON_NOT_CONFIRMED);
            continue;
        }

        led_set_blink_now(true, 50);
        
        if(control_button_confirmed_press(3000)){
            led_set_blink_now(false, 50);
            continue;
        }

        oled_show_motor_calibrating(true);
        motor_calibrate();
        oled_show_motor_calibrating(false);

        led_set_blink_now(false, 50);

        while(control_button_is_pressed()){
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        while(ulTaskNotifyTake(pdTRUE, 0) > 0){
            ESP_LOGW(__func__, "Discarded spurious button trigger during hold.\n");
        }
    }
}

void pilot_task(void *pvParameter){
    while(1){
        xTaskNotifyWait(0, UINT32_MAX, NULL, portMAX_DELAY);

        float current_speed = get_current_speed();
        float current_throttle_position = tps_get_throttle_position();

        if(autopilot_state != AUTOPILOT_DISABLED){
            oled_show_message(OLED_MESSAGE_AUTOPILOT_BUSY);
            continue;
        }

        if(!control_button_confirmed_press(1000)){
            oled_show_message(OLED_MESSAGE_BUTTON_NOT_CONFIRMED);
            continue;
        }
        
        if(brake_sensor_is_pressed()){
            oled_show_message(OLED_MESSAGE_BRAKE_ACTIVE);
            ESP_LOGI(__func__, "Auto pilot not enabled because the brake is pressed.\n");
            
            continue;
        }
        
        // if(clutch_is_actuated()){
            //     oled_show_message(OLED_MESSAGE_CLUTCH_ACTIVE);
            //     ESP_LOGI(__func__, "Auto pilot not enabled because the clutch is actuated.\n");
            //     continue;
            // }
            
            if(current_speed < MINIMUM_SPEED_LIMIT){
            oled_show_message(OLED_MESSAGE_SPEED_TOO_LOW);
            ESP_LOGI(__func__, "Auto pilot not enabled because the speed is below %d km/h.\n", MINIMUM_SPEED_LIMIT);
            
            continue;
        }

        if(!auto_pilot_is_active()){
            enable_auto_pilot(current_speed, current_throttle_position);
        }else{
            disable_auto_pilot();
        }

        while(control_button_is_pressed()){
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        while(ulTaskNotifyTake(pdTRUE, 0) > 0){
            ESP_LOGW(__func__, "Discarded spurious button trigger during hold.\n");
        }
    }
}

void cruise_control_task(void *pvParameter){
    int64_t previous_time = esp_timer_get_time();

    while(1){
        int64_t current_time = esp_timer_get_time();
        float elapsed_time_s = (current_time - previous_time) / 1000000.0f;

        previous_time = current_time;

        if(auto_pilot_is_active()){
            float current_speed = get_current_speed();
            float acceleration_error = speed_target - current_speed;
            float pid_output = pid_calculate(acceleration_error, elapsed_time_s);

            motor_set_output(pid_output);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void enable_auto_pilot(float speed, float throttle_position){
    autopilot_state = AUTOPILOT_ACTIVATING;

    motor_pre_accelerate(throttle_position);
    
    speed_target = speed;
    
    led_turn_on();

    autopilot_state = AUTOPILOT_ACTIVE;
    
    ESP_LOGI(__func__, "Speed target set to %.0f km/h.", speed);
    ESP_LOGI(__func__, "Auto pilot enabled.\n");
}

void disable_auto_pilot(){
    autopilot_state = AUTOPILOT_DEACTIVATING;

    speed_target = 0;

    reset_pid();
    motor_return_throttle_to_idle();
    led_turn_off();

    autopilot_state = AUTOPILOT_DISABLED;

    ESP_LOGI(__func__, "Auto pilot disabled.\n");
}

bool auto_pilot_is_active(){
    return autopilot_state == AUTOPILOT_ACTIVE;
}

float get_speed_target(){
    return speed_target;
}

void cruise_control_init(){
    motor_return_throttle_to_idle();

    xTaskCreate(pilot_task, "pilot_task", 2048, NULL, 4, &pilot_task_handle);
    xTaskCreate(calibration_task, "calibration_task", 2048, NULL, 4, &calibration_task_handle);
    xTaskCreate(emergency_release_task, "emergency_release_task", 2048, NULL, 4, &emergency_release_task_handle);
    xTaskCreate(cruise_control_task, "cruise_control_task", 2048, NULL, 3, NULL);

    ESP_LOGI(__func__, "Cruise control initialized successfully.");
}