#include "pid.h"
#include <stdint.h>
#include "config.h"

static float integral = 0;
static float last_error = 0;

float kp(float err){
    return err * PID_KP;
}

float ki(float err){
    integral += err;
    return integral * PID_KI;
}

float kd(float err){
    float der = PID_KD * (err - last_error);
    last_error = err;
    return der;
}

void reset_pid(void){
    integral = 0;
    last_error = 0;
}

float pid_calculate(float err){
    float prop = kp(err);
    float inte = ki(err);
    float deri = kd(err);
    
    float output = prop + inte + deri;
    
    return output;
}