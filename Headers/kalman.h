#pragma once

void init_temp_kstate(float q, float r, float x, float p, float k);
void init_acc_kstate(float q, float r, float x, float p, float k);
float k_filter_temp(float measurement);
float k_filter_acc(float measurement);
