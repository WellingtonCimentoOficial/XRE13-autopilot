#pragma once

void motor_set_throttle(float throttle_position);
void motor_return_throttle_to_idle(void);
void motor_set_throttle_to_max(void);
void motor_set_output(float pid_output);
void motor_calibrate(void);
void motor_init(void);