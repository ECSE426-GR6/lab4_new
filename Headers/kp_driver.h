#pragma once

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "mail_controller.h"

#define LATCH_CONFIRM_COUNT 3

#define KP_PORT GPIOC

#define ROW_0_PIN GPIO_PIN_6
#define ROW_1_PIN GPIO_PIN_7
#define ROW_2_PIN GPIO_PIN_8
#define ROW_3_PIN GPIO_PIN_9

#define COL_0_PIN GPIO_PIN_10
#define COL_1_PIN GPIO_PIN_11
#define COL_2_PIN GPIO_PIN_12

void KP_update(void);
int KP_getValueDown(void);
int KP_getEvent(void);
void KP_init(void);
int KEYPAD_start_thread(void);
