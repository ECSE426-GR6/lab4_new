#include "stm32f4xx_hal.h"
#include "Temperature.h"

ADC_HandleTypeDef g_AdcHandle;

/**
 * Configure adc and temp channel
 */
void ConfigureADC(void)
{
		ADC_ChannelConfTypeDef adcChannel;

    __ADC1_CLK_ENABLE();

    // HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
    // HAL_NVIC_EnableIRQ(ADC_IRQn);

    g_AdcHandle.Instance = ADC1;

    // g_AdcHandle.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;
    g_AdcHandle.Init.Resolution = ADC_RESOLUTION_12B;
    g_AdcHandle.Init.ScanConvMode = DISABLE;
    g_AdcHandle.Init.ContinuousConvMode = ENABLE;
    // g_AdcHandle.Init.DiscontinuousConvMode = DISABLE;
    // g_AdcHandle.Init.NbrOfDiscConversion = 0;
    g_AdcHandle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    g_AdcHandle.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
    g_AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    g_AdcHandle.Init.NbrOfConversion = 1;
    // g_AdcHandle.Init.DMAContinuousRequests = ENABLE;
    // g_AdcHandle.Init.EOCSelection = DISABLE;

    HAL_ADC_Init(&g_AdcHandle);


    adcChannel.Channel = ADC_CHANNEL_TEMPSENSOR;
    adcChannel.Rank = 1;
    adcChannel.SamplingTime = ADC_SAMPLETIME_144CYCLES;
    adcChannel.Offset = 0;

    HAL_ADC_ConfigChannel(&g_AdcHandle, &adcChannel);

		HAL_ADC_Start(&g_AdcHandle);
}


//Convert adc voltage to degrees celcius
float convertVtoC(int voltage){
	float convert = (float)voltage;
	convert *= 3000; //MV max voltage
	convert /= 0xfff; //mv
	convert *= 0.001f; //volt
	convert -= 0.770f; //Ref voltage for temp
	convert *= 400.0f; //slope
	convert += 25.0f; //offset

	return convert;
}

/**
 * Get current cpu temp
 * @return  cpu temp in degrees celcius
 */
float getTemp(void){
	return convertVtoC(HAL_ADC_GetValue(&g_AdcHandle));
}
