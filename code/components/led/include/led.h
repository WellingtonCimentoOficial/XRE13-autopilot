#pragma once

#include <stdbool.h>

void led_init(void);
void led_set_blink_now(bool state, float speed);
void led_turn_on(void);
void led_turn_off(void);
