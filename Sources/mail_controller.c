#include "mail_controller.h"

float temperature = 0.0f;
float angle = 0.0f;

int key_event = -1;
int target_set = 0;

MAIL_package_type curent_display_type = MAIL_TEMP;

//Mailbox sensors and keypad will use to send data to controller
osMailQDef(input_mailbox, 5, MAIL_package_typedef);
osMailQId  input_mailbox;

//Mailbox controller will use send data to led
osMailQDef(led_mailbox, 5, MAIL_package_typedef);
osMailQId  led_mailbox;


void MAIL_CONTROLLER_init_mailboxes(void) {
	input_mailbox = osMailCreate(osMailQ(input_mailbox), NULL);
	led_mailbox = osMailCreate(osMailQ(led_mailbox), NULL);
}

osMailQId input_mailbox_id(void){
	return input_mailbox;
}

osMailQId led_mailbox_id(void){
	return led_mailbox;
}

void MAIL_send_input(MAIL_package_type type, float value) {
	MAIL_package_typedef *package;
	
	package = osMailAlloc(input_mailbox, osWaitForever);
	package->value = value;
	package->type = type;
	osMailPut(input_mailbox, package);
}

///RTOS THREAD FUNCTIONS -----------------------------------------
void Thread_MAIL_CONTROLLER(void const *argument);					// thread function
osThreadId tid_Thread_MAIL_CONTROLLER;                              // thread id
osThreadDef(Thread_MAIL_CONTROLLER, osPriorityAboveNormal, 1, 0);	//Above normal because it must respond to the three data inputs (temp, angle and keypad)

int MAIL_CONTROLLER_start_thread(void) {
	tid_Thread_MAIL_CONTROLLER = osThreadCreate(osThread(Thread_MAIL_CONTROLLER), NULL); // Start LED_Thread

	if (!tid_Thread_MAIL_CONTROLLER) return (-1); 
	return (0);
}


void Thread_MAIL_CONTROLLER(void const *argument) {
	osEvent event;
	MAIL_package_typedef *package_recieved;
	MAIL_package_typedef *package_to_send;
	
	while (1) {
		event = osMailGet(input_mailbox, osWaitForever);
		
		if (event.status == osEventMail) {
			package_recieved = (MAIL_package_typedef *) event.value.p;
			
			if (package_recieved->type == MAIL_TEMP) { //Update temp
				temperature = package_recieved->value;
				printf("Message recieved from main");
			} else if (package_recieved->type == MAIL_ANGLE) { //Update angle
				angle = package_recieved->value;
				
			} else if (package_recieved->type == MAIL_KEY) { //Update key event
				key_event = (int)package_recieved->value;
				
				if (key_event == KP_STAR) { //Star pressed
					//Change which type of event we send to led
					if (curent_display_type == MAIL_TEMP) {
						curent_display_type = MAIL_ANGLE;
						
						//Prepare package to send current angle value to led right away at last if of function
						package_recieved->type = curent_display_type;
						package_recieved->value = angle;
						
					} else if (curent_display_type == MAIL_ANGLE) {
						curent_display_type = MAIL_TEMP;
						
						//Prepare package to send current temp value to led right away at last if of function
						package_recieved->type = curent_display_type;
						package_recieved->value = temperature;
					}
					
				} else if (key_event == KP_POUND) { //Pound pressed
					if (!target_set) {
						target_set = 1;
						
						//Send pound event to led to confirm current target value
						package_to_send = osMailAlloc(led_mailbox, osWaitForever);
						package_to_send->value = key_event;
						package_to_send->type = MAIL_KEY;
						osMailPut(led_mailbox, package_to_send);
					}
					
				} else { //Digit pressed
					if (!target_set) {
						
						//Send key event to led to change current digit
						package_to_send = osMailAlloc(led_mailbox, osWaitForever);
						package_to_send->value = key_event;
						package_to_send->type = MAIL_KEY;
						osMailPut(led_mailbox, package_to_send);
						
						printf("Message sent from controller to led");
					}
				}
			}
			
			//Send new data to LED if of current type 
			if (package_recieved->type == curent_display_type){ 
				package_to_send = osMailAlloc(led_mailbox, osWaitForever);
				package_to_send->value = package_recieved->value;
				package_to_send->type = package_recieved->type;
				osMailPut(led_mailbox, package_to_send);
			}
			
			osMailFree(input_mailbox, package_recieved);
		}
	}
}
