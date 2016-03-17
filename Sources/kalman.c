#include "kalman.h"

typedef struct StateVars{
	float q;
	float r;
	float x;
	float p;
	float k;
} kalman_state;

kalman_state temp_kstate;
kalman_state acc_kstate;

void init_temp_kstate(float q, float r, float x, float p, float k){
		temp_kstate.q = q;
    temp_kstate.r = r;
    temp_kstate.x = x;
    temp_kstate.p = p;
    temp_kstate.k = k;
}

void init_acc_kstate(float q, float r, float x, float p, float k){
		acc_kstate.q = q;
    acc_kstate.r = r;
    acc_kstate.x = x;
    acc_kstate.p = p;
    acc_kstate.k = k;
}

float k_filter_temp(float measurement){
	temp_kstate.p = temp_kstate.p + temp_kstate.q;
	temp_kstate.k = temp_kstate.p  / (temp_kstate.p + temp_kstate.r);
	temp_kstate.x = temp_kstate.x + temp_kstate.k * ( measurement - temp_kstate.x);
	temp_kstate.p = (1 - temp_kstate.k) * temp_kstate.p;
	return temp_kstate.x;
}

float k_filter_acc(float measurement){
	acc_kstate.p = acc_kstate.p + acc_kstate.q;
	acc_kstate.k = acc_kstate.p  / (acc_kstate.p + acc_kstate.r);
	acc_kstate.x = acc_kstate.x + acc_kstate.k * ( measurement - acc_kstate.x);
	acc_kstate.p = (1 - acc_kstate.k) * acc_kstate.p;
	return acc_kstate.x;
}

