#pragma once

void reset_pid(void);
float pid_calculate(float err, float elapsed_time_s);
void set_kp(float kp);
float get_kp();
void set_ki(float ki);
float get_ki();
void set_kd(float kd);
float get_kd();