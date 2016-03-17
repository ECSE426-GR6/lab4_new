#include "lis3dsh.h"
#include <Math.h>
#include "Accelerometer.h"
#define PI 3.14159265359

int data_ready = 0;
float Rangle(void);
float calculateangle(float x, float y, float z);

void accelerometer_init(void){

	GPIO_InitTypeDef GPIO_InitStruct;
	
	LIS3DSH_InitTypeDef meminit;
	LIS3DSH_DRYInterruptConfigTypeDef intinit;

	meminit.Continous_Update = LIS3DSH_ContinousUpdate_Disabled;
	meminit.Power_Mode_Output_DataRate = LIS3DSH_DATARATE_25;
	meminit.AA_Filter_BW = LIS3DSH_AA_BW_800;
	meminit.Axes_Enable = LIS3DSH_XYZ_ENABLE;
	meminit.Full_Scale = LIS3DSH_FULLSCALE_2;
	meminit.Self_Test = LIS3DSH_SELFTEST_NORMAL;
  
	LIS3DSH_Init(&meminit);

	intinit.Dataready_Interrupt = LIS3DSH_DATA_READY_INTERRUPT_ENABLED;
	intinit.Interrupt_signal = LIS3DSH_ACTIVE_HIGH_INTERRUPT_SIGNAL;
	intinit.Interrupt_type = LIS3DSH_INTERRUPT_REQUEST_PULSED;
  
	LIS3DSH_DataReadyInterruptConfig(&intinit);

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	
	NVIC_SetPriority(EXTI0_IRQn, 1);
	NVIC_EnableIRQ(EXTI0_IRQn);
}



float Rangle(){

float xyz[3];
float xangle;
	float yangle;
	float zangle;
	 LIS3DSH_ReadACC(xyz);
	
	// X offset ~= -19
	// Y offset ~=  +12
	// Z offset ~=  -40
	// the offset is added to the read value for calibration
	
	 xangle = xyz[0] -19;
	 yangle = xyz[1] +12;
		zangle = xyz[2] -40 ;
	

	
	
	return calculateangle(xangle,yangle,zangle);
	
}

//the calculate angle function to compute the pitch and roll angles
float calculateangle(float x, float y, float z){
float pitch;
float roll;
float angle;

 pitch = atan(x / (sqrt(y*y+z*z)))*(180/PI) +90 ;
// + 90 degrees is added to each angle to demonstrate that the sensor works both directions.
	// cannot display negative
	roll = atan(y / (sqrt(x*x+z*z)))*(180/PI) +90 ;
	
	return angle = pitch;
}