#pragma once

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

#define KP_STAR 10
#define KP_POUND 11

#define ALARM_THRESHOLD 40

typedef enum {
	MAIL_TEMP = 0,      
	MAIL_ANGLE = 1,
	MAIL_KEY = 2,
	MAIL_ALARM = 3
} MAIL_package_type;

typedef struct {
	float value;
	MAIL_package_type type;
} MAIL_package_typedef;
	
void MAIL_CONTROLLER_init_mailboxes(void);
int MAIL_CONTROLLER_start_thread(void);
osMailQId input_mailbox_id(void);
osMailQId led_mailbox_id(void);
void MAIL_send_input(MAIL_package_type type, float value);

