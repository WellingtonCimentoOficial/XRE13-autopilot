#include "pid.h"
#include <stdint.h>
#include "config.h"

static float integral = 0;
static float last_error = 0;

static float _kp = 10.0f;
static float _ki = 0;
static float _kd = 20.0f;
 
void set_kp(float kp){
    _kp = kp;
}

float get_kp(){
    return _kp;
}

void set_ki(float ki){
    _ki = ki;
}

float get_ki(){
    return _ki;
}

void set_kd(float kd){
    _kd = kd;
}

float get_kd(){
    return _kd;
}

float kp(float err){
    return err * _kp;
}

float ki(float err){
    integral += err;
    return integral * _ki;
}

float kd(float err, float elapsed_time_s){
    float der = _kd * ((err - last_error) / elapsed_time_s);
    last_error = err;
    return der;
}

void reset_pid(void){
    integral = 0;
    last_error = 0;
}

float pid_calculate(float err, float elapsed_time_s){
    float prop = kp(err);
    float inte = ki(err);
    float deri = kd(err, elapsed_time_s);
    
    float output = prop + inte + deri;
    
    return output;
}