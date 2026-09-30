#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ssd1306.h"
#include "font8x8_basic.h"
#include "config.h"
#include <stdbool.h>

static SSD1306_t dev;

static bool was_initialized = false;

void oled_print(char *text, int line){
    if(was_initialized){
        ssd1306_display_text_x3(&dev, line, text, strlen(text), false);
    }
}

void oled_print_two_lines(char *text1, char *text2){
    if(was_initialized){
        ssd1306_display_text_x3(&dev, 0, text1, strlen(text1), false);
        ssd1306_display_text_x3(&dev, 5, text2, strlen(text2), false);
    }
}

void oled_init(void){
	i2c_master_init(&dev, I2C_MASTER_SDA_GPIO, I2C_MASTER_SCL_GPIO, -1);
    
    dev._flip = true;
    
    ssd1306_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);

    was_initialized = true;

    ESP_LOGI(__func__, "Oled initialized successfully!");
}