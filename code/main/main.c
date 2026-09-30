#include <stdio.h>
#include "gpio_hall.h"
#include "cruise_control.h"
#include "motor_control.h"
#include "tps_sensor.h"
#include "speed_sensor.h"
#include "clutch_sensor.h"
#include "brake_sensor.h"
#include "control_button.h"
#include "driver/gpio.h"
#include "oled.h"
#include "led.h"
#include "adc_hall.h"
#include "esp_adc/adc_oneshot.h"

void app_main(void)
{
    gpio_install_isr_service(0);

    adc_oneshot_unit_handle_t adc_handle = adc_init();

    control_button_init();
    brake_sensor_init();
    clutch_sensor_init();
    tps_sensor_init(adc_handle);
    motor_init();
    led_init();
    oled_init();
    speed_sensor_init();
    cruise_control_init();
}