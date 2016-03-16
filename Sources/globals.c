#include "globals.h"

float temperature = 0.0f;
float angle = 0.0f;
float kp_in = 0.0f;

void GLO_setTemperature(float newTemperature){
	temperature = newTemperature;
}

float GLO_getTemperature(void){
	return temperature;
}

void GLO_setAngle(float newAngle){
	angle = newAngle;
}

float GLO_getAngle(void){
	return angle;
}

void GLO_set_KP_in(float new_kp_in){
	kp_in = new_kp_in;
}

float GLO_get_KP_in(void){
	return kp_in;
}
