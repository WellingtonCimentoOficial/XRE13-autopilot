#include "pwm_hall.h"

esp_err_t pwm_init(int gpio_num, pwm_channel_t channel){
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = PWM_MODE,
        .duty_resolution  = PWM_DUTY_RESOULTION,
        .timer_num        = PWM_TIMER,
        .freq_hz          = PWM_FREQUENCY_INITIAL,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    esp_err_t timer_config_result = ledc_timer_config(&ledc_timer);
    if(timer_config_result != ESP_OK)
        return timer_config_result;

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = PWM_MODE,
        .channel        = channel,
        .timer_sel      = PWM_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = gpio_num,
        .duty           = PWM_DUTY_INITIAL,
        .hpoint         = 0
    };
    esp_err_t channel_config_result = ledc_channel_config(&ledc_channel);
    if(channel_config_result != ESP_OK)
        return channel_config_result;

    return ESP_OK;
}
esp_err_t pwm_set_duty(pwm_channel_t channel, int duty){
    esp_err_t set_duty_result = ledc_set_duty(PWM_MODE, channel, duty);
    if(set_duty_result != ESP_OK)
        return set_duty_result;

    esp_err_t update_duty_result =  ledc_update_duty(PWM_MODE, channel);
    if(update_duty_result != ESP_OK)
        return update_duty_result;

    return ESP_OK;
}
esp_err_t pwm_set_freq(double frequency) {
    esp_err_t set_freq_result = ledc_set_freq(PWM_MODE, PWM_TIMER, frequency);
    return set_freq_result;
}