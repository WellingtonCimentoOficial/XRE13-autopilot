#include "control_button.h"
#include "gpio_hall.h"
#include "config.h"
#include "cruise_control.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdbool.h>

static volatile uint32_t btn_control_last_interrupt = 0;

void IRAM_ATTR btn_control_isr(void *args){
    uint32_t now = xTaskGetTickCountFromISR();

    if(btn_control_last_interrupt == 0 || ((now - btn_control_last_interrupt) > pdMS_TO_TICKS(BTN_CONTROL_DEBOUNCE_TIME))){
        btn_control_last_interrupt = now;
        xTaskNotifyFromISR(get_pilot_task_handle(), 0, eNoAction, NULL);
        xTaskNotifyFromISR(get_calibration_task_handle(), 0, eNoAction, NULL);
    }
}

bool control_button_is_pressed(void){
    return gpio_get(BTN_CONTROL_GPIO) == GPIO_OFF;
}

bool control_button_confirmed_press(uint32_t time_in_ms){
    TickType_t start = xTaskGetTickCount();

    ESP_LOGI(__func__, "Button press confirmation initiated.");

    while(control_button_is_pressed()){
        TickType_t elapsed = xTaskGetTickCount() - start;

        if(elapsed >= pdMS_TO_TICKS(time_in_ms)){
            ESP_LOGI(__func__, "Pressing the confirmed button.");
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    return false;
}

void control_button_init(){
    gpio_init(BTN_CONTROL_GPIO, GPIO_MODE_INPUT, GPIO_PULLUP_ENABLE, GPIO_PULLDOWN_DISABLE, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(BTN_CONTROL_GPIO, btn_control_isr, NULL);

    ESP_LOGI(__func__, "Control button initialized successfully.");
}

