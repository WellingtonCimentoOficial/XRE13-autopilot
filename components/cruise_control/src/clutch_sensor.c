#include "clutch_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "gpio_hall.h"
#include "config.h"
#include "cruise_control.h"
#include <stdbool.h>

static volatile uint32_t cluch_sensor_last_interrupt = 0;

static TaskHandle_t clutch_sensor_task_handle;

void IRAM_ATTR clutch_sensor_isr(void *args){
    uint32_t now = xTaskGetTickCountFromISR();

    if(cluch_sensor_last_interrupt == 0 || ((now - cluch_sensor_last_interrupt) > pdMS_TO_TICKS(CLUCH_SENSOR_DEBOUNCE_TIME))){
        cluch_sensor_last_interrupt = now;
        xTaskNotifyFromISR(clutch_sensor_task_handle, 0, eNoAction, NULL);
    }
}

bool clutch_is_actuated(void){
    return gpio_get(CLUTCH_SENSOR_GPIO) == GPIO_ON;
}

void clutch_sensor_task(void *pvParameter){
    while(1){
        xTaskNotifyWait(0, UINT32_MAX, NULL, portMAX_DELAY);

        ESP_LOGI(__func__, "ACTUATED CLUTCH\n");

        if(auto_pilot_is_active()){
            disable_auto_pilot();
        }

        while(clutch_is_actuated()){
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        while(ulTaskNotifyTake(pdTRUE, 0) > 0){
            ESP_LOGW(__func__, "Discarded spurious button trigger during hold.\n");
        }
    }
}

void clutch_sensor_init(void){
    gpio_init(CLUTCH_SENSOR_GPIO, GPIO_MODE_INPUT, GPIO_PULLUP_DISABLE, GPIO_PULLDOWN_DISABLE, GPIO_INTR_POSEDGE);

    xTaskCreate(clutch_sensor_task, "cluch_sensor_task", 4096, NULL, 5, &clutch_sensor_task_handle);

    gpio_isr_handler_add(CLUTCH_SENSOR_GPIO, clutch_sensor_isr, NULL);

    ESP_LOGI(__func__, "Clutch sensor initialized successfully.");

}