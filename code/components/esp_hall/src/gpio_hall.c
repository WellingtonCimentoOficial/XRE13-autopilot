#include "gpio_hall.h"

esp_err_t gpio_init(int gpio_num, gpio_mode_t mode, gpio_pullup_t pullup, gpio_pulldown_t pulldown, gpio_int_type_t intr){
    gpio_config_t io_config = {
        .pin_bit_mask = (1ULL << gpio_num),
        .mode = mode,
        .pull_up_en = pullup,
        .pull_down_en = pulldown,
        .intr_type = intr
    };
    esp_err_t config_result = gpio_config(&io_config);
    if(config_result != ESP_OK)
        return config_result;

    return ESP_OK;
}
esp_err_t gpio_set(int gpio_num, gpio_control_t level){
    esp_err_t result = gpio_set_level(gpio_num, level);
    return result;
}
gpio_control_t gpio_get(int gpio_num){
    int result = gpio_get_level(gpio_num);
    if(result == 1)
        return GPIO_ON;
    return GPIO_OFF;
}