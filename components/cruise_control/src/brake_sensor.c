#include "brake_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "gpio_hall.h"
#include "config.h"
#include "cruise_control.h"
#include <stdbool.h>

static volatile uint32_t brake_sensor_last_interrupt = 0;

static TaskHandle_t brake_sensor_task_handle;

void IRAM_ATTR brake_sensor_isr(void *args){
    uint32_t now = xTaskGetTickCountFromISR();

    if(brake_sensor_last_interrupt == 0 || ((now - brake_sensor_last_interrupt) > pdMS_TO_TICKS(BRAKE_SENSOR_DEBOUNCE_TIME))){
        brake_sensor_last_interrupt = now;
        xTaskNotifyFromISR(brake_sensor_task_handle, 0, eNoAction, NULL);
    }
}

bool brake_sensor_is_pressed(void){
    return gpio_get(BRAKE_SENSOR_GPIO) == GPIO_OFF;
}

void brake_sensor_task(void *pvParameter){
    while(1){
        xTaskNotifyWait(0, UINT32_MAX, NULL, portMAX_DELAY);

        ESP_LOGI(__func__, "BRAKE APPLIED\n");

        if(auto_pilot_is_active()){
            disable_auto_pilot();
        }

        while(brake_sensor_is_pressed()){
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        while(ulTaskNotifyTake(pdTRUE, 0) > 0){
            ESP_LOGW(__func__, "Discarded spurious button trigger during hold.\n");
        }
    }
}

void brake_sensor_init(void){
    gpio_init(BRAKE_SENSOR_GPIO, GPIO_MODE_INPUT, GPIO_PULLUP_DISABLE, GPIO_PULLDOWN_DISABLE, GPIO_INTR_NEGEDGE);

    xTaskCreate(brake_sensor_task, "brake_sensor_task", 4096, NULL, 5, &brake_sensor_task_handle);
    
    gpio_isr_handler_add(BRAKE_SENSOR_GPIO, brake_sensor_isr, NULL);

    ESP_LOGI(__func__, "Brake sensor initialized successfully.");

}