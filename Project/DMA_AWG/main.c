#include "stm32f4xx.h"
#include "misc.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main(void)
{
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 
    initADC();
    initTimer();
    initLED();
    int index = 0;
    uint16_t ADCBuffer[16];
    memset(ADCBuffer, 0, sizeof(uint16_t));
    volatile uint16_t avgReading = 0;

    while(1)
    {
	while(  ADC_GetFlagStatus( ADC1, ADC_FLAG_EOC) == RESET );
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	ADCBuffer[index] = ADC_GetConversionValue(ADC1);
	index++;
	if(index == 16)
	{
	    index = 0;

	    avgReading = 0;
	    for(int i = 0; i < 16; i++ )
	    {
		avgReading += ADCBuffer[i];
	    }
	    if( avgReading > 300 )
	    {
		GPIO_SetBits(GPIOD, GPIO_Pin_14);
	    }
	    else
	    {
		GPIO_ResetBits(GPIOD, GPIO_Pin_14);
	    }
	}
    }

    return 0;
}

void initADC(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef GPIOInit;
    GPIOInit.GPIO_Pin = GPIO_Pin_3;
    GPIOInit.GPIO_Mode = GPIO_Mode_AN;
    GPIOInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIOInit);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    ADC_InitTypeDef ADCInit;
    ADC_StructInit( &ADCInit );
    ADCInit.ADC_Resolution = ADC_Resolution_12b;
    ADCInit.ADC_ScanConvMode = DISABLE;
    ADCInit.ADC_ContinuousConvMode = ENABLE;
    ADCInit.ADC_ExternalTrigConv = ADC_ExternalTrigConvEdge_None;
    ADCInit.ADC_DataAlign = ADC_DataAlign_Right;
    ADCInit.ADC_NbrOfConversion = 1;

    ADC_CommonInitTypeDef ADCCommonInit;
    ADC_CommonStructInit( &ADCCommonInit );
    ADCCommonInit.ADC_Mode = ADC_Mode_Independent;
    ADCCommonInit.ADC_Prescaler = ADC_Prescaler_Div8;

    ADC_Init(ADC1, &ADCInit);
    ADC_CommonInit( &ADCCommonInit );
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1, ADC_SampleTime_480Cycles);
    ADC_Cmd(ADC1, ENABLE);
    ADC_SoftwareStartConv(ADC1);

}

void initLED(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitTypeDef GPIOInit;
    GPIOInit.GPIO_Pin = GPIO_Pin_14;
    GPIOInit.GPIO_Mode = GPIO_Mode_OUT;
    GPIOInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIOInit);
}

void initTimer(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIOInit;
    GPIOInit.GPIO_Pin = GPIO_Pin_10;
    GPIOInit.GPIO_Mode = GPIO_Mode_AF;
    GPIOInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIOInit);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef timerInitStructure;
    timerInitStructure.TIM_Prescaler = 5;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = 10;
    timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    timerInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &timerInitStructure);
    TIM_Cmd(TIM2, ENABLE);

   TIM_OCInitTypeDef outputChannelInit = {0,};
    outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
    outputChannelInit.TIM_Pulse = 5;
    outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
    outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC3Init(TIM2, &outputChannelInit);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_TIM2);
}
