/**
 * Functions to control and display the led 4 digit 7 segment display as well as the 4 led alarm
 * new stuff
 */

#include "led_driver.h"

int digit_mask(int digit);
int led_segments(int value, int* segments);
int write_digit(int value, int digit);
int write_float(float value);
void LED_setPins(void);
void LED_set_value(float);
void LED_update(void);
float change_digit(float, int);

//Clock display variables
float current_value = 10.0f;
float target_value = 0.0f;

//State values
int digit_mode = 1; //True: showing numbers : False: Showing direction to tilt
int moving_decimal_mode = 0; //True : changing decimal to chow more precise numbers : False : always 1 decimal value
int decimal_digit = 1; //Which digit decimal is on
int degree_mode = 0; //If degree light is on
int blinking = 0; //Alarm blinking
int target_was_set = 0; //If angle target was set once

int target_input_digit = 3;

int blinking_count = 0;
const int BLINK_PERIODS = 50;
const int MAX_BLINK_COUNT = BLINK_PERIODS * 10;
const int HALF_BLINK_COUNT = BLINK_PERIODS * 5;

int current_digit = 3;
int digits[5] = {cathode0, cathode1, cathode2, cathode3, cathode4};
int pins[7] = {pinA, pinB, pinC, pinD, pinE, pinF, pinG};

int digit_segments[4][7] = {
	{0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}
	};

int alarm_led = 0;
int alarm_count;
int alarm_pins[4] = {ALARM_UP, ALARM_LEFT, ALARM_DOWN, ALARM_RIGHT};


///RTOS FUNCTIONS -----------------------------------------
void Thread_LED (void const *argument);                 // thread function
osThreadId tid_Thread_LED;                              // thread id
osThreadDef(Thread_LED, osPriorityNormal, 1, 0);


MAIL_package_type display_mode = MAIL_ANGLE;

/**
 * Start led thread
 * @return  success
 */
int LED_start_thread(void){
	tid_Thread_LED = osThreadCreate(osThread(Thread_LED ), NULL); // Start LED_Thread

  	if (!tid_Thread_LED) return(-1);
  	return(0);
}

//LED thread behaviour
void Thread_LED (void const *argument) {
	int key = -1;

	osEvent event;
	osMailQId  led_mailbox = led_mailbox_id();
	MAIL_package_typedef *package_recieved;

	while(1){
		event = osMailGet(led_mailbox, osWaitForever);

		if (event.status == osEventMail) {
			package_recieved = (MAIL_package_typedef *) event.value.p;

			if (package_recieved->type == MAIL_TEMP) { //Recieved temp data
				if (display_mode != MAIL_TEMP) {
					display_mode = MAIL_TEMP;
					LED_switch_temp();
				}
				LED_set_value(package_recieved->value);

			} else if (package_recieved->type == MAIL_ANGLE) { //Recieved angle data
				if (display_mode != MAIL_ANGLE) {
					display_mode = MAIL_ANGLE;
					LED_switch_angle();
				}

				if (target_was_set) LED_set_value(package_recieved->value);
				else LED_set_value(target_value);

			} else if (package_recieved->type == MAIL_KEY) { //Recieved key input
				if (display_mode == MAIL_ANGLE){
					key = (int)package_recieved->value;
					if (key == 11) {
						LED_set_target();
					} else if (key > -1 && key < 10){
						target_value = change_digit(target_value, key);
						LED_set_value(target_value);
					}
				}
			}else if (package_recieved->type == MAIL_ALARM) { //Alarm toggle
				if(package_recieved->value) LED_alarm_on();
				else LED_alarm_off();
			}
		}
		osMailFree(led_mailbox, package_recieved);
	}
}

//Change one digit in a float number (used for target angle input)
float change_digit(float number, int new_val){
  int base_int = (int)(number * 10);

  int dec = base_int % 10;
  int one = (base_int / 10) % 10;
  int ten = (base_int / 100) % 10;
  int hundred = (base_int / 1000) % 10;

	target_input_digit = (target_input_digit + 1) % 4;

  if (target_input_digit == 3) {
    float decimal = ((float)dec) / 10;
    number -= decimal;
    decimal = ((float)new_val) / 10;
    number += decimal;
    return number;
  }
  if (target_input_digit == 2) {
    return number - one + new_val;
  }
  if (target_input_digit == 1) {
    return number - (ten * 10) + (new_val * 10);
  }
  if (target_input_digit == 0) {
    return number - (hundred * 100) + (new_val * 100);
  }

  return -1;
}

/**
* Set the target angle for player to reach
* @param target target value
*/
void LED_set_target(void){
	digit_mode = 0;
	moving_decimal_mode = 1;
	target_was_set = 1;
}

/**
 * Switch to displaying temp values
 */
void LED_switch_temp(void){
	digit_mode = 1;
	moving_decimal_mode = 0;
	decimal_digit = 1;
	degree_mode = 0;
}

/**
 * switch to displaying angle / target input
 */
void LED_switch_angle(void){
	if (target_was_set){
		digit_mode = 0;
		moving_decimal_mode = 1;
	} else {
		digit_mode = 1;
		moving_decimal_mode = 0;
		decimal_digit = 1;
		degree_mode = 0;
	}
}

/**
 * set alarm on
 */
void LED_alarm_on(void){
	blinking = 1;
}

/**
 * set alarm off
 */
void LED_alarm_off(void){
	blinking = 0;
	HAL_GPIO_WritePin(ALARM_PORT, alarm_pins[alarm_led], GPIO_PIN_RESET);
}

//Switch to next alarm led
void switch_alarm_light(){
	HAL_GPIO_WritePin(ALARM_PORT, alarm_pins[alarm_led], GPIO_PIN_RESET);
	alarm_led = (alarm_led + 1) % 4;
	HAL_GPIO_WritePin(ALARM_PORT, alarm_pins[alarm_led], GPIO_PIN_SET);
}

/**
 * init led gpio
 */
void LED_init_io (void){
	GPIO_InitTypeDef LED_configuration;


	__HAL_RCC_GPIOE_CLK_ENABLE();

	LED_configuration.Pin	= GPIO_PIN_1 | GPIO_PIN_10 | GPIO_PIN_5 | GPIO_PIN_2 | GPIO_PIN_8 | GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_15 | GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_10;
	LED_configuration.Mode 	= GPIO_MODE_OUTPUT_PP;
	LED_configuration.Pull	= GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &LED_configuration);

	__HAL_RCC_GPIOD_CLK_ENABLE();

	LED_configuration.Pin	= GPIO_PIN_15 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
	LED_configuration.Mode 	= GPIO_MODE_OUTPUT_PP;
	LED_configuration.Pull	= GPIO_NOPULL;
	HAL_GPIO_Init(GPIOD, &LED_configuration);

}
///BEHAVIOUR FUNCTIONS ------------------------------------
/**
* Switches the current digit being lit
*/
void LED_update(void){
	if (blinking){
		if (alarm_count > ALARM_LED_SWITCH_COUNT){
			switch_alarm_light();
			alarm_count = 0;
		}
		alarm_count++;


		blinking_count = (blinking_count + 1) % MAX_BLINK_COUNT;
		if (blinking_count < HALF_BLINK_COUNT) {
			HAL_GPIO_WritePin(LED_PORT, digits[current_digit], GPIO_PIN_RESET);
			return;
		}
	}

	HAL_GPIO_WritePin(LED_PORT, digits[current_digit], GPIO_PIN_RESET);
	current_digit = (current_digit + 1) % 5;
	LED_setPins();
	HAL_GPIO_WritePin(LED_PORT, digits[current_digit], GPIO_PIN_SET);
}

//set the segment pins for current digit
void LED_setPins(){
	if (current_digit == 4){
		HAL_GPIO_WritePin(LED_PORT, pinA, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_PORT, pinB, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_PORT, pinC, degree_mode ? GPIO_PIN_SET : GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_PORT, pinD, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_PORT, pinE, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_PORT, pinF, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_PORT, pinG, GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(LED_PORT, pinA, digit_segments[current_digit][0] == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_PORT, pinB, digit_segments[current_digit][1] == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_PORT, pinC, digit_segments[current_digit][2] == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_PORT, pinD, digit_segments[current_digit][3] == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_PORT, pinE, digit_segments[current_digit][4] == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_PORT, pinF, digit_segments[current_digit][5] == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_PORT, pinG, digit_segments[current_digit][6] == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);

		if (current_digit == decimal_digit)
		{
			HAL_GPIO_WritePin(LED_PORT, pinDP, GPIO_PIN_SET);
		} else {
			HAL_GPIO_WritePin(LED_PORT, pinDP, GPIO_PIN_RESET);
		}
	}
}



/**
 * Set the private temp value to be displayed on next refresh
 * @param new_value new value to set temp to
 */
void LED_set_value(float new_value){
	int base_int;

	if (digit_mode){
		base_int = (int)(new_value * 10);
		current_value = new_value;



		//keypad input mode
		if (!moving_decimal_mode){
			led_segments(base_int % 10, digit_segments[0]);
			led_segments((base_int / 10) % 10, digit_segments[1]);
			led_segments((base_int / 100) % 10, digit_segments[2]);
			led_segments((base_int / 1000) % 10, digit_segments[3]);
		} else { //Angle display mode
			degree_mode = 1;

			if (new_value > target_value + 5 || new_value < target_value - 5){
				digit_mode = 0;
			}

			if (new_value > 99.9f) {
				decimal_digit = 4;
				led_segments(11, digit_segments[0]);
				led_segments((base_int / 10) % 10, digit_segments[1]);
				led_segments((base_int / 100) % 10, digit_segments[2]);
				led_segments((base_int / 1000) % 10, digit_segments[3]);
			} else if (new_value > 9.99f){
				decimal_digit = 2;
				base_int = (int)(new_value * 100);
				led_segments(11, digit_segments[0]);
				led_segments((base_int / 10) % 10, digit_segments[1]);
				led_segments((base_int / 100) % 10, digit_segments[2]);
				led_segments((base_int / 1000) % 10, digit_segments[3]);
			} else {
				decimal_digit = 3;
				base_int = (int)(new_value * 1000);
				led_segments(11, digit_segments[0]);
				led_segments((base_int / 10) % 10, digit_segments[1]);
				led_segments((base_int / 100) % 10, digit_segments[2]);
				led_segments((base_int / 1000) % 10, digit_segments[3]);
			}
		}
	} else { //Angle direction mode
			degree_mode = 0;
		if (new_value < target_value + 5 && new_value > target_value - 5){
			digit_mode = 1;
		} else if (new_value < target_value){
			led_segments(11, digit_segments[0]);
			led_segments(11, digit_segments[1]);
			led_segments(10, digit_segments[2]);
			if (target_value - new_value > 50){
				led_segments(10, digit_segments[3]);
			} else {
				led_segments(11, digit_segments[3]);
			}
		} else {
			led_segments(11, digit_segments[2]);
			led_segments(11, digit_segments[3]);
			led_segments(10, digit_segments[1]);
			if (new_value - target_value> 50){
				led_segments(10, digit_segments[0]);
			} else {
				led_segments(11, digit_segments[0]);
			}
		}
	}
}

/**
 * Get current private temp value
 * @return  current temp
 */
float LED_get_value(void){
	return current_value;
}

/**
 * Returns an array of the segments to switch on ([0] is A, [6] is G)
 * @param  value    int value to convert
 * @param  segments array to store result in
 * @return          error code (0 if no errors)
 */
int led_segments(int value, int* segments){
	int mask = digit_mask(value);
	segments[0] = mask & segA;
	segments[1] = mask & segB;
	segments[2] = mask & segC;
	segments[3] = mask & segD;
	segments[4] = mask & segE;
	segments[5] = mask & segF;
	segments[6] = mask & segG;

	return mask;
}

int digit_mask(int value) {
	if (value == 0) return dig0;
	if (value == 1) return dig1;
	if (value == 2) return dig2;
	if (value == 3) return dig3;
	if (value == 4) return dig4;
	if (value == 5) return dig5;
	if (value == 6) return dig6;
	if (value == 7) return dig7;
	if (value == 8) return dig8;
	if (value == 9) return dig9;
	if (value == 10) return mid_bar;
	if (value == 11) return blank;

	return -1;
}
