#pragma once

#include "driver/gpio.h"
#include "esp_err.h"

typedef enum {GPIO_OFF, GPIO_ON} gpio_control_t;

esp_err_t gpio_init(int gpio_num, gpio_mode_t mode, gpio_pullup_t pullup, gpio_pulldown_t pulldown, gpio_int_type_t intr);
esp_err_t gpio_set(int gpio_num, gpio_control_t level);
gpio_control_t gpio_get(int gpio_num);