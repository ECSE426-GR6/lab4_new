/*******************************************************************************
  * @file    main.c
  * @author  Ashraf Suyyagh
	* @version V1.2.0
  * @date    17-January-2016
  * @brief   This file demonstrates flasing one LED at an interval of one second
	*          RTX based using CMSIS-RTOS 
  ******************************************************************************
  */
#include "cmsis_os.h"   
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
                // ARM::CMSIS:RTOS:Keil RTX
#include "RTE_Components.h"             // Component selection
#include "led_driver.h"
#include "Accelerometer.h"
#include "Temperature.h"
extern void initializeLED_IO			(void);
extern void start_Thread_LED			(void);
extern void Thread_LED(void const *argument);
extern osThreadId tid_Thread_LED;
void accelerometer_thread(void const *argument);


const int ACC_DATA_READY_SIGNAL = 10;

void accelerometer_thread(void const *argument);
int accelerometer_thread_start(void);

osThreadId accelerometer_thread_id;
osThreadDef(accelerometer_thread, osPriorityNormal, 1, 0);

const int TEMP_DATA_READY_SIGNAL = 20;
void temperature_thread(void const *argument);
int temperature_start_thread(void);

osThreadId temperature_thread_id;
osThreadDef(temperature_thread, osPriorityNormal, 1, 0);

void TIM3_init(void);
void TIM4_init(void);
void testFuntion(void);

/**
	These lines are mandatory to make CMSIS-RTOS RTX work with te new Cube HAL
*/
#ifdef RTE_CMSIS_RTOS_RTX
extern uint32_t os_time;

uint32_t HAL_GetTick(void) { 
  return os_time; 
}
#endif

/**
  * System Clock Configuration
  */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the
     device is clocked below the maximum system frequency (see datasheet). */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

/**
  * Main function
  */
int main (void) {

	osKernelInitialize();                     /* initialize CMSIS-RTOS          */

	HAL_Init();                               /* Initialize the HAL Library     */

	SystemClock_Config();                     /* Configure the System Clock     */

	/* User codes goes here*/
	// initializeLED_IO();                       /* Initialize LED GPIO Buttons    */
	// start_Thread_LED();                       /* Create LED thread              */
 
	MAIL_CONTROLLER_init_mailboxes();
	ConfigureADC();
	LED_init_io();
	TIM3_init();
	TIM4_init();
	
	MAIL_CONTROLLER_start_thread();
	LED_start_thread();
	temperature_start_thread();
	/* User codes ends here*/
  
	osKernelStart();                          /* start thread execution         */
	
	testFuntion();
}



TIM_HandleTypeDef TIM_Handle;
TIM_HandleTypeDef TIM_Handle2;
void TIM3_init(void){
__TIM3_CLK_ENABLE();
		TIM_Handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4; // 168 MHz / 4 = 42 MHz
    TIM_Handle.Init.Prescaler = 419; // 42 MHz / (419 + 1) = 100 KHz
    TIM_Handle.Init.Period = 499; // 100 KHz / (499 + 1) = 200 Hz
    TIM_Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
    TIM_Handle.Instance = TIM3;   //Same timer whose clocks we enabled
    HAL_TIM_Base_Init(&TIM_Handle);     // Init timer
    HAL_TIM_Base_Start_IT(&TIM_Handle); // start timer interrupts
    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void TIM4_init(void){
__TIM4_CLK_ENABLE();
		TIM_Handle2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4; // 168 MHz / 4 = 42 MHz
    TIM_Handle2.Init.Prescaler = 419; // 42 MHz / (419 + 1) = 100 KHz
    TIM_Handle2.Init.Period = 9999; // 100 KHz / (999 + 1) = 100 Hz
    TIM_Handle2.Init.CounterMode = TIM_COUNTERMODE_UP;
    TIM_Handle2.Instance = TIM4;   //Same timer whose clocks we enabled
    HAL_TIM_Base_Init(&TIM_Handle2);     // Init timer
    HAL_TIM_Base_Start_IT(&TIM_Handle2); // start timer interrupts
    HAL_NVIC_SetPriority(TIM4_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
}

void TIM3_IRQHandler(void)
{
    if (__HAL_TIM_GET_FLAG(&TIM_Handle, TIM_FLAG_UPDATE) != RESET)      //In case other interrupts are also running
    {
        if (__HAL_TIM_GET_ITSTATUS(&TIM_Handle, TIM_IT_UPDATE) != RESET)
        {
            __HAL_TIM_CLEAR_FLAG(&TIM_Handle, TIM_FLAG_UPDATE);
            LED_update();
        }
    }
}

void TIM4_IRQHandler(void)
{
    if (__HAL_TIM_GET_FLAG(&TIM_Handle2, TIM_FLAG_UPDATE) != RESET)      //In case other interrupts are also running
    {
        if (__HAL_TIM_GET_ITSTATUS(&TIM_Handle2, TIM_IT_UPDATE) != RESET)
        {
            __HAL_TIM_CLEAR_FLAG(&TIM_Handle2, TIM_FLAG_UPDATE);
						
						osSignalSet(temperature_thread_id, TEMP_DATA_READY_SIGNAL);
					
        }
    }
}





void accelerometer_thread(void const *argument){
while (1){
	osSignalWait(ACC_DATA_READY_SIGNAL, osWaitForever);
	
		MAIL_send_input(MAIL_TEMP, Rangle()); //Change MAIL_TEMP to MAIL_ANGLE
	
	osSignalClear(accelerometer_thread_id, ACC_DATA_READY_SIGNAL);
	
}
}

int temperature_start_thread(void){
	temperature_thread_id = osThreadCreate(osThread(temperature_thread ), NULL); // Start LED_Thread

  	if (!temperature_thread_id) return(-1); 
  	return(0);
}

void temperature_thread(void const *argument) {
	float temp;
	
	while (1){
		osSignalWait(TEMP_DATA_READY_SIGNAL, osWaitForever);
		
		temp = getTemp();
		printf("%f\n",temp);
		MAIL_send_input(MAIL_TEMP, temp);
		
		osSignalClear(temperature_thread_id, ACC_DATA_READY_SIGNAL);
	}
}

void testFuntion(void){
	osMailQId input_mailbox = input_mailbox_id();
	MAIL_package_typedef *package_to_send;
	
	package_to_send = osMailAlloc(input_mailbox, osWaitForever);
	package_to_send->value = 877.3f;
	package_to_send->type = MAIL_TEMP;
	osMailPut(input_mailbox, package_to_send);

}
