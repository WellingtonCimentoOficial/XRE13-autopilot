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
#include "tps_sensor.h"
#include "speed_sensor.h"
#include "cruise_control.h"
#include "oled.h"

static SSD1306_t dev;

static bool was_initialized = false;
static oled_error_t oled_error = OLED_ERROR_NONE;
static bool oled_motor_calibrating = false;

static char* oled_get_message();
// static void oled_print(char *text, int line, oled_font_t font);

typedef enum {
    OLED_MODE_OFF,
    OLED_MODE_ACTIVE,
    OLED_MODE_CALIBRATING,
    OLED_MODE_ERROR
} oled_mode_t;

void oled_task(void *arg){
    oled_mode_t current_mode;
    oled_mode_t last_mode = OLED_MODE_OFF;

    while(1){
        float throttle_position = tps_get_throttle_position();
        float current_speed = get_current_speed();
        float speed_target = get_speed_target();

        char buffer[17];

        if(oled_motor_calibrating){
            current_mode = OLED_MODE_CALIBRATING;
        }
        
        else if(oled_error != OLED_ERROR_NONE){
            current_mode = OLED_MODE_ERROR;
        }
        
        else if(auto_pilot_is_active()){
            current_mode = OLED_MODE_ACTIVE;
        }
        
        else{
            current_mode = OLED_MODE_OFF;
        }

        if(current_mode != last_mode){
            ssd1306_clear_screen(&dev, false);
            last_mode = current_mode;
        }

        if(current_mode == OLED_MODE_CALIBRATING){
            oled_print("CALIB...", 0, OLED_FONT_MEDIUM);

            snprintf(buffer, sizeof(buffer), "TPS %3.0f%%", throttle_position);
            oled_print(buffer, 6, OLED_FONT_MEDIUM);

            vTaskDelay(pdMS_TO_TICKS(10));

            continue;
        }
        
        else if(current_mode == OLED_MODE_ERROR){
            oled_print(" ALERT ", 0, OLED_FONT_MEDIUM);

            oled_print("   |   ", 2, OLED_FONT_MEDIUM);
            oled_print("   |   ", 3, OLED_FONT_MEDIUM);
            oled_print("   V   ", 4, OLED_FONT_MEDIUM);
            
            oled_print(oled_get_message(), 6, OLED_FONT_MEDIUM);

            vTaskDelay(pdMS_TO_TICKS(3000));

            oled_error = OLED_ERROR_NONE;
        }
        
        else if(current_mode == OLED_MODE_ACTIVE){
            snprintf(buffer, sizeof(buffer), "TGT %3.0f", speed_target);
            oled_print(buffer, 0, OLED_FONT_MEDIUM);

            snprintf(buffer, sizeof(buffer), "TPS %3.0f%%", throttle_position);
            oled_print(buffer, 3, OLED_FONT_MEDIUM);

            snprintf(buffer, sizeof(buffer), "%3.0f km/h", current_speed);
            oled_print(buffer, 6, OLED_FONT_MEDIUM);
        }
        
        else{
            oled_print("AUTO OFF", 0, OLED_FONT_MEDIUM);

            snprintf(buffer, sizeof(buffer), "%3.0f km/h", current_speed);
            oled_print(buffer, 6, OLED_FONT_MEDIUM);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static char* oled_get_message(){
    switch (oled_error){
    case OLED_ERROR_NOT_READY:
        return "Ready";
    case OLED_ERROR_BUTTON_NOT_CONFIRMED:
        return "Button";
    case OLED_ERROR_SPEED_TOO_LOW:
        return "Speed";
    case OLED_ERROR_BRAKE_ACTIVE:
        return "Brake";
    case OLED_ERROR_CLUTCH_ACTIVE:
        return "Clutch";
    default:
        return "";
    }
}
void oled_show_error(oled_error_t error){
    oled_error = error;
}

void oled_show_motor_calibrating(bool show){
    oled_motor_calibrating = show;
}

void oled_print(char *text, int line, oled_font_t font){
    if(!was_initialized){
        return;
    }

    if(font == OLED_FONT_SMALL){
        ssd1306_display_text(&dev, line, text, strlen(text), false);
    }else if(font == OLED_FONT_MEDIUM){
        ssd1306_display_text_x2(&dev, line, text, strlen(text), false);
    }else{
        ssd1306_display_text_x3(&dev, line, text, strlen(text), false);
    }
}

void oled_init(void){
	i2c_master_init(&dev, I2C_MASTER_SDA_GPIO, I2C_MASTER_SCL_GPIO, -1);
    
    dev._flip = true;
    
    ssd1306_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);

    was_initialized = true;

    xTaskCreate(oled_task, "oled_task", 2048, NULL, 3, NULL);

    ESP_LOGI(__func__, "Oled initialized successfully!");
}