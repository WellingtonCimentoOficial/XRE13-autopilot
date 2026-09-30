#pragma once

#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"

#define PWM_TIMER              LEDC_TIMER_0
#define PWM_MODE               LEDC_LOW_SPEED_MODE
#define PWM_DUTY_RESOULTION    LEDC_TIMER_8_BIT
#define PWM_DUTY_INITIAL       0
#define PWM_FREQUENCY_INITIAL  20000

typedef enum {
    PWM_CHANNEL_O,
    PWM_CHANNEL_1,
    PWM_CHANNEL_2,
    PWM_CHANNEL_3,
    PWM_CHANNEL_4,
    PWM_CHANNEL_5,
} pwm_channel_t;

esp_err_t pwm_init(int gpio_num, pwm_channel_t channel);
esp_err_t pwm_set_duty(pwm_channel_t channel, int duty);
esp_err_t pwm_set_freq(double frequency);