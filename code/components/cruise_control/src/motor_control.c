#include "motor_control.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdint.h>
#include "pwm_hall.h"
#include "config.h"
#include <math.h>
#include "tps_sensor.h"
#include "led.h"

void motor_open_throttle(float duty){
    pwm_set_duty(MOTOR_DECELERATION_PWM_CHANNEL, 0);

    pwm_set_duty(MOTOR_ACCELERATION_PWM_CHANNEL, duty);
    ESP_LOGI(__func__, "Acceleration PWM set. Current duty: %.2f.", duty);
}
void motor_close_throttle(float duty){
    pwm_set_duty(MOTOR_ACCELERATION_PWM_CHANNEL, 0);

    pwm_set_duty(MOTOR_DECELERATION_PWM_CHANNEL, duty);
    ESP_LOGI(__func__, "Deceleration PWM set. Current duty: %.2f.", duty);
}

void motor_stop(void){
    motor_open_throttle(0);
    motor_close_throttle(0);
    ESP_LOGI(__func__, "Accelerate duty and decelerate duty set to 0.");
}

float limit_pwm(float pid_output){
    uint32_t pwm_limit = (1 << PWM_DUTY_RESOULTION) - 1;

    pid_output = fabsf(pid_output);

    if(pid_output > pwm_limit){
        return pwm_limit;
    }

    return pid_output;
}

void motor_return_throttle_to_idle(void){
    if(!tps_is_throttle_at_idle()){
        ESP_LOGI(__func__, "Throttle is not at idle (current: %.2f%%). Resetting position...", tps_get_throttle_position());
        
        float last_correct_throttle_position = tps_get_throttle_position();
        uint32_t last_correct_throttle_position_time = xTaskGetTickCount();
        
        while(1){
            while(!tps_is_throttle_at_idle()){
                motor_close_throttle(255);
    
                if(last_correct_throttle_position < tps_get_throttle_position()){
                    motor_stop();
    
                    while(last_correct_throttle_position < tps_get_throttle_position()){
                        vTaskDelay(pdMS_TO_TICKS(10));
                        continue;
                    }
    
                    last_correct_throttle_position_time = xTaskGetTickCount();
    
                    while((xTaskGetTickCount() - last_correct_throttle_position_time) < pdMS_TO_TICKS(500)){
                        vTaskDelay(pdMS_TO_TICKS(10));
                        continue;
                    }
    
                    last_correct_throttle_position = tps_get_throttle_position();
                }
    
                ESP_LOGI(__func__, "Resetting throttle: current position %.2f%% -> target 0%%.", tps_get_throttle_position());
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            vTaskDelay(pdMS_TO_TICKS(500));

            if(tps_is_throttle_at_idle()){
                break;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(300));
        
        motor_stop();

        ESP_LOGI(__func__, "Throttle successfully reset to 0%%.\n");
    }
}

void motor_set_throttle_to_max(void){ // to test
    if(!tps_is_throttle_at_max()){
        ESP_LOGI(__func__, "Throttle is not in max (current: %.2f%%). setting max position...", tps_get_throttle_position());
        motor_open_throttle(255);
        while(!tps_is_throttle_at_max()){
            ESP_LOGI(__func__, "Setting throttle: current position %.2f%% -> target 100%%.", tps_get_throttle_position());
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        motor_stop();
        ESP_LOGI(__func__, "Throttle successfully set to 100%%.\n");
    }
}

void motor_calibrate(void){
    motor_set_throttle_to_max();
    motor_return_throttle_to_idle();
}

void motor_set_throttle(float throttle_position){ 
    ESP_LOGI(__func__, "Waiting for throttle to be released...");
    led_set_blink_now(true, 50);
    
    while(!tps_is_throttle_at_idle()){
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    ESP_LOGI(__func__, "Throttle released.");
    
    if(tps_get_throttle_position() < throttle_position){
        ESP_LOGI(__func__, "Setting throttle to %.2f%%...", throttle_position);
        led_set_blink_now(true, 200);

        motor_open_throttle(255);
        while(tps_get_throttle_position() < throttle_position){
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        motor_stop();
        ESP_LOGI(__func__, "Throttle successfully set to %.2f%%.\n", throttle_position);
    }

    led_set_blink_now(false, 200);
}


void motor_set_output(float pid_output){
    float duty = limit_pwm(pid_output);

    if(pid_output > 0.0f && !tps_is_throttle_at_max()){
        motor_open_throttle(duty);
    }else if(pid_output < 0.0f && !tps_is_throttle_at_idle()){
        motor_close_throttle(duty);
    }else{
        motor_stop();
    }
}

void motor_init(){
    pwm_init(MOTOR_ACCELERATION_PWM_GPIO, MOTOR_ACCELERATION_PWM_CHANNEL);
    pwm_init(MOTOR_DECELERATION_PWM_GPIO, MOTOR_DECELERATION_PWM_CHANNEL);

    ESP_LOGI(__func__, "Motor initialized successfully.");
}