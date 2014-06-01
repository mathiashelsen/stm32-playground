/* Copyright 2014 Mathias Helsen */

#include "stm32f4xx.h"
#include "misc.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "usart.h"
#include "dac.h"
#include "adc.h"
#include "timer.h"

#define STATE_IDLE		(int32_t) 0
#define STATE_ACTIVE		(int32_t) 1
#define STATE_START		(int32_t) 2
#define STATE_FORCE		(int32_t) 3

volatile struct 
{
    uint16_t start;
    uint16_t step;
    uint16_t stop;
} DAC1, DAC2;

volatile int32_t state;
volatile uint16_t DAC1Value;
volatile uint16_t DAC2Value;

volatile uint16_t *ADCBuffer;
volatile uint32_t ADCIndex;
volatile uint32_t EOL;
volatile uint32_t EOS;

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
    init_USART();
    state = STATE_IDLE;
    EOL = 0;
    EOS = 0;

    DAC1.start	= 0x0000;
    DAC1.step	= 0x0020;
    DAC1.stop	= 0x1000;
    DAC1Value	= 0x0000;

    DAC2.start	= 0x0000;
    DAC2.step	= 0x0200;
    DAC2.stop	= 0x1000;
    DAC2Value	= 0x0000;

    ADCBuffer	= malloc(128 * sizeof(uint16_t));

    while(1)
    {
	if(state == STATE_FORCE)
	{
	    TIM_Cmd(TIM2, ENABLE);
	    state = STATE_START;
	}
    }
}

void TIM2_IRQHandler(void)
{
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    if( state == STATE_START )
    {
	ADCIndex = 0;
	DAC1Value = DAC1.start;
	DAC2Value = DAC2.start;
	DAC_SetChannel1Data(DAC_Align_12b_R, DAC1Value);
	DAC_SetChannel2Data(DAC_Align_12b_R, DAC2Value);
	state = STATE_ACTIVE;
    }
    else
    {
	DAC1Value += DAC1.step;
    
	if( DAC1Value > DAC1.stop )
	{
	    EOL = 1;
	    DAC1Value = DAC1.start;
	    DAC2Value += DAC2.step;
	    if(DAC2Value > DAC2.stop)
	    {
		DAC2Value = DAC2.start;
		EOS = 1;
	    }
	    DAC_SetChannel2Data(DAC_Align_12b_R, DAC2Value);
	}
	DAC_SetChannel1Data(DAC_Align_12b_R, DAC1Value);
    }
}

void ADC_IRQHandler(void)
{
    ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
    if( state != STATE_ACTIVE )
	return;

    ADCBuffer[ADCIndex] = ADC_GetConversionValue(ADC1);
    // Check if we are at the end of the V_CE sweep
    if( EOL )
    {
	// Disable the timer temporary
	TIM_Cmd(TIM2, DISABLE);
	// Transmit the data
	uint16_t nBytes = sizeof(uint16_t)*(ADCIndex);
	transmit( 2, &nBytes);
	transmit( sizeof(uint16_t)*(ADCIndex), (uint8_t*) ADCBuffer);	
	// Reset the index
	ADCIndex = 0;
	// Check if we are at the end of the V_BE sweep
	// if so, don't restart the timer
	if( !EOS )
	{
	    EOL = 0;
	    TIM_Cmd(TIM2, ENABLE);
	}
	else
	{
	    DAC_SetChannel1Data(DAC_Align_12b_R, DAC1.start);
	    DAC_SetChannel2Data(DAC_Align_12b_R, DAC2.start);
	    state = STATE_IDLE;
	    nBytes = 0;
	    transmit( 2, &nBytes);
	}
    }
    else
    { 
	ADCIndex++;
    }
}

void USART3_IRQHandler(void)
{
    USART_ClearFlag(USART3, USART_IT_RXNE);
    USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
    // See if the first byte is the magic word
    if ( 0xA3 == USART3->DR )
    {
	DAC1.start = readHalfword();
	DAC1.step = readHalfword();
	DAC1.stop = readHalfword();

	DAC2.start = readHalfword();
	DAC2.step = readHalfword();
	DAC2.stop = readHalfword();

	state = STATE_START;
	// Reset to the initial state
	free( (void *) ADCBuffer);
    	volatile uint32_t nSamples = 1 + (uint32_t) ((DAC1.stop - DAC1.start)/DAC1.step);
	ADCBuffer	= malloc(nSamples * sizeof(uint16_t));
	ADCIndex	= 0;
	EOL = 0;
	EOS = 0;

	// Enable TIM2   
	TIM_Cmd(TIM2, ENABLE);
    }
    USART_ClearFlag(USART3, USART_IT_RXNE);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
}


