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
#include "oled.h"
#include "led.h"
#include "esp_timer.h"

static float speed_target = 0;
static bool is_ready = false;
// static bool btn_control_is_pressed = false;
static bool is_active = false;

TaskHandle_t control_task_handle;

TaskHandle_t get_control_task_handle(void){
    return control_task_handle;
}

void control_task(void *pvParameter){
    while(1){
        xTaskNotifyWait(0, UINT32_MAX, NULL, portMAX_DELAY);

        oled_print(".     ", 0);
        
        float current_speed = get_current_speed();
        float current_throttle_position = tps_get_throttle_position();

        // if(btn_control_is_pressed || !is_ready){
        //     continue;
        // }

        if(!is_ready){
            oled_print("READY", 0);
            continue;
        }

        if(!control_button_confirmed_press()){
            oled_print("BUTTON", 0);
            continue;
        }

        // btn_control_is_pressed = true;

        if(brake_sensor_is_pressed()){
            oled_print("BRAKE", 0);
            ESP_LOGI(__func__, "Auto pilot not enabled because the brake is pressed.\n");
            continue;
        }

        // if(clutch_is_actuated()){
        //     oled_print("ACT", 0);
        //     ESP_LOGI(__func__, "Auto pilot not enabled because the clutch is actuated.\n");
        //     continue;
        // }

        if(current_speed < MINIMUM_SPEED_LIMIT){
            oled_print("SPEED", 0);
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

        // btn_control_is_pressed = false;
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

            // float throttle_position = tps_get_throttle_position();

            // char buffer[7];
            // snprintf(buffer, sizeof(buffer), "%-3.2f", throttle_position);

            // oled_print(buffer, 0);

            motor_set_output(pid_output);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void enable_auto_pilot(float speed, float throttle_position){
    motor_set_throttle(throttle_position);
    
    speed_target = speed;
    is_active = true;
    
    char buffer[7];
    snprintf(buffer, sizeof(buffer), "%-3.0f", speed_target);
    oled_print(buffer, 0);

    led_turn_on();
    
    ESP_LOGI(__func__, "Speed target set to %.0f km/h.", speed);
    ESP_LOGI(__func__, "Auto pilot enabled.\n");
}

void disable_auto_pilot(){
    is_active = false;
    speed_target = 0;
    is_ready = false;

    reset_pid();
    motor_return_throttle_to_idle();
    led_turn_off();

    oled_print("   ", 0);

    is_ready = true;

    ESP_LOGI(__func__, "Auto pilot disabled.\n");
}

bool auto_pilot_is_active(){
    return is_active;
}

float get_speed_target(){
    return speed_target;
}

void cruise_control_init(){
    // if(get_current_speed() == 0.0f){
    //     motor_set_throttle_to_max();
    // }
    motor_return_throttle_to_idle();

    xTaskCreate(control_task, "control_task", 4096, NULL, 4, &control_task_handle);
    xTaskCreate(cruise_control_task, "cruise_control_task", 2048, NULL, 3, NULL);

    is_ready = true;

    ESP_LOGI(__func__, "Cruise control initialized successfully.");
}