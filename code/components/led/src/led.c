#include "led.h"
#include "gpio_hall.h"
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"

static bool blink_led_now = false;
static float blink_speed = 100;
static gpio_control_t led_state = GPIO_OFF;

void led_set_blink_now(bool state, float speed){
    blink_led_now = state;
    blink_speed = speed;
}

void led_turn_on(void){
    led_state = GPIO_ON;
    gpio_set(LED_GPIO, led_state);
}

void led_turn_off(void){
    led_state = GPIO_OFF;
    gpio_set(LED_GPIO, led_state);
}

void blink_led(void){
    while(blink_led_now){
        led_state = !led_state;
        gpio_set(LED_GPIO, led_state);
        vTaskDelay(pdMS_TO_TICKS(blink_speed));
    }
}

void blink_led_task(void *pvParameter){
    while(1){
        blink_led();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void led_init(void){
    gpio_init(LED_GPIO, GPIO_MODE_OUTPUT, GPIO_PULLUP_ENABLE, GPIO_PULLDOWN_DISABLE, GPIO_INTR_DISABLE);

    xTaskCreate(blink_led_task, "blink_led_task", 4096, NULL, 4, NULL);
}