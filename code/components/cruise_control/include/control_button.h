#pragma once

#include <stdbool.h>
#include <stdint.h>

bool control_button_is_pressed(void);
bool control_button_confirmed_press(uint32_t time_in_ms);
void control_button_init(void);