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
    float current_throttle_position = tps_get_throttle_position();
    if(current_throttle_position > ACCELERATOR_MINIMUM_TOLERANCE){
        ESP_LOGI(__func__, "Throttle is not at idle (current: %.2f%%). Resetting position...", current_throttle_position);
        motor_close_throttle(255);
        while(current_throttle_position > ACCELERATOR_MINIMUM_TOLERANCE){
            ESP_LOGI(__func__, "Resetting throttle: current position %.2f%% -> target 0%%.", current_throttle_position);
            current_throttle_position = tps_get_throttle_position();
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
        motor_stop();
        ESP_LOGI(__func__, "Throttle successfully reset to 0%%.\n");
    }
}

void motor_set_throttle_to_max(void){ // to test
    float current_throttle_position = tps_get_throttle_position();
    if(current_throttle_position < ACCELERATOR_MAXIMUM_TOLERANCE){
        ESP_LOGI(__func__, "Throttle is not in max (current: %.2f%%). setting max position...", current_throttle_position);
        motor_open_throttle(255);
        while(current_throttle_position < ACCELERATOR_MAXIMUM_TOLERANCE){
            ESP_LOGI(__func__, "Setting throttle: current position %.2f%% -> target 100%%.", current_throttle_position);
            current_throttle_position = tps_get_throttle_position();
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        motor_stop();
        ESP_LOGI(__func__, "Throttle successfully set to 100%%.\n");
    }
}

void motor_set_throttle(float throttle_position){ 
    float current_throttle_position = tps_get_throttle_position();

    ESP_LOGI(__func__, "Waiting for throttle to be released...");
    led_set_blink_now(true, 100);

    while(current_throttle_position > ACCELERATOR_MINIMUM_TOLERANCE){
        current_throttle_position = tps_get_throttle_position();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    ESP_LOGI(__func__, "Throttle released.");

    if(current_throttle_position < throttle_position){
        ESP_LOGI(__func__, "Setting throttle to %.2f%%...", throttle_position);
        led_set_blink_now(true, 200);

        motor_open_throttle(255);
        while(current_throttle_position < throttle_position){
            current_throttle_position = tps_get_throttle_position();
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        motor_stop();
        ESP_LOGI(__func__, "Throttle successfully set to %.2f%%.\n", throttle_position);
    }

    led_set_blink_now(false, 200);
}


void motor_set_output(float pid_output){
    float current_throttle_position = tps_get_throttle_position();
    float duty = limit_pwm(pid_output);

    if(pid_output > 0 && current_throttle_position < ACCELERATOR_MAXIMUM_TOLERANCE){
        motor_open_throttle(duty);
    }else if(pid_output < 0 && current_throttle_position > ACCELERATOR_MINIMUM_TOLERANCE){
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