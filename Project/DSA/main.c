#include "stm32f4xx.h"
#include "misc.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <usart.h>
#include <adc.h>

#define TIM_PERIOD 839 // (42MHz/50kHz - 1)
volatile uint32_t   enableScan;
volatile uint32_t   adcIndex;
volatile uint32_t   samples;
volatile int16_t   *adcBuffer;

void init_TIM2(void); // TIM2 running at 1MHz

int main(void)
{
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 
    samples = 50000;
    adcBuffer	= malloc(samples * sizeof(int16_t));
    adcIndex = samples+1;

    init_TIM2();
    TIM_Cmd(TIM2, DISABLE);
    init_ADC();
    init_USART();

    while(1)
    {
	if( adcIndex == samples )
	{
	    TIM_Cmd(TIM2, DISABLE);
	    uint32_t nBytes = sizeof(int16_t) * samples;
	    transmit(4, (volatile uint8_t *)&nBytes);
	    transmit(nBytes, (volatile uint8_t *)adcBuffer );
	    adcIndex++;
	}
    }

}

void TIM2_IRQHandler(void)
{
    if( TIM_GetITStatus( TIM2, TIM_IT_Update) != RESET )
    {
	TIM_ClearITPendingBit( TIM2, TIM_IT_Update );
	// Read a value from the ADC and add it to the register
	if(adcIndex < samples)
	{
	    readChannel(0, &(adcBuffer[adcIndex]));
	    adcIndex++;
	}
    }
}

void init_TIM2(void)
{
    // Enable the clock to the timer 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // APB1 clock: 168MHz/4 = 42 MHz

    TIM_TimeBaseInitTypeDef TIMInit = {0, }; 
    TIMInit.TIM_Prescaler   = 1;
    TIMInit.TIM_CounterMode = TIM_CounterMode_Up;
    TIMInit.TIM_Period	    = TIM_PERIOD; // running at 1 MHz
    TIMInit.TIM_ClockDivision	= TIM_CKD_DIV1;
    //TIM_SelectOnePulseMode(TIM2, TIM_OPMode_Single);	// Only one pulse, for debugging purposes

    // Init but DON'T enable
    TIM_TimeBaseInit(TIM2, &TIMInit );

    // Trigger output on update
    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);

    // Enable interrupt
    TIM_ITConfig( TIM2, TIM_IT_Update, ENABLE);
    TIM_ClearITPendingBit( TIM2, TIM_IT_Update);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // Higher priority than TIM3_IRQn
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void USART3_IRQHandler(void)
{
    USART_ClearFlag(USART3, USART_IT_RXNE);
    USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
    // See if the first byte is the magic word
    if ( 0xA3 == USART3->DR )
    {
	// Reset to the initial state
	adcIndex	= 0;

	// Enable TIM2   
	TIM_Cmd(TIM2, ENABLE);
    }
    USART_ClearFlag(USART3, USART_IT_RXNE);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
}
