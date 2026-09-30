#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void enable_auto_pilot(float speed, float throttle_position);
void disable_auto_pilot(void);
bool auto_pilot_is_active(void);
float get_speed_target(void);
TaskHandle_t get_control_task_handle(void);
void cruise_control_init(void);