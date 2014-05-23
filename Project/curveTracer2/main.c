/* Copyright 2014 Mathias Helsen */

#include "stm32f4xx.h"
#include "misc.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "usart.h"
#include "dac.h"
#include "adc.h"
#include "timer.h"

#define STATE_IDLE		(int32_t) 0
#define STATE_ACTIVE		(int32_t) 1
#define STATE_TRANSMITTING	(int32_t) 2

volatile struct 
{
    uint16_t start;
    uint16_t step;
    uint16_t stop;
} DAC1, DAC2;

volatile int32_t state;

volatile uint16_t *ADC1Buffer;
volatile uint32_t adcIndex;

/*
 * This is the deal, once started:
 * Tim2 will trigger every 50us, this will update the DAC(s)
 * 25us later Tim3 will start a conversion for ADC1
 * ADC1 triggers an interrupt when it has finished conversion
 *
 * When the ADC has gathered enough samples, the timers will be halted
 * and transmission will start of the trace, when this is not the last trace
 * another trace will be recorded.
 */
int main(void)
{
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 
    init_DACs();
    init_ADC1();
    init_Timers();
    state = STATE_IDLE;
    while(1)
    {
    }
}

void TIM2_IRQHandler(void)
{
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
//DAC_Align_12b_R
//void DAC_SetChannel1Data(uint32_t DAC_Align, uint16_t Data);
//void DAC_SetChannel2Data(uint32_t DAC_Align, uint16_t Data);
}

void ADC_IRQHandler(void)
{
    ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
//uint16_t ADC_GetConversionValue(ADC_TypeDef* ADCx);
}

void USART3_IRQHandler(void)
{
    USART_ClearFlag(USART3, USART_IT_RXNE);
    // See if the first byte is the magic word
    if (0xA3 & USART3->DR)
    {
	DAC1.start = readHalfword();
	DAC1.step = readHalfword();
	DAC1.stop = readHalfword();

	DAC2.start = readHalfword();
	DAC2.step = readHalfword();
	DAC2.stop = readHalfword();

	state = STATE_ACTIVE;
	// Reset to the initial state

	// Enable TIM2   
	TIM_Cmd(TIM2, ENABLE);
    }
}


