#include <stm32f4xx.h>
#include <stdlib.h>

#include "clock.h"

void init_clock(int ADC_PERIOD, int SAMPLES) {
	// Enable the clock to the timer
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // APB1 clock: 168MHz/4 = 42 MHz

	TIM_TimeBaseInitTypeDef TIMInit = {0, };
	TIMInit.TIM_Prescaler     = 1;
	TIMInit.TIM_CounterMode   = TIM_CounterMode_Up;
	TIMInit.TIM_Period	      = ADC_PERIOD;
	TIMInit.TIM_ClockDivision = TIM_CKD_DIV1;

	// Init but DON'T enable
	TIM_TimeBaseInit(TIM2, &TIMInit );

	// Trigger output on update
	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);

	// Enable clock to TIM3
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIMInit.TIM_Period        = SAMPLES-1; // Wait for number of samples to be aqcuired

	// Init
	TIM_TimeBaseInit(TIM3, &TIMInit );
	TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Trigger);	// TIM3 will be a slave to TIM2
	TIM_ITRxExternalClockConfig(TIM3, TIM_TS_ITR1);

	TIM_Cmd(TIM3, ENABLE);
	TIM_ClearITPendingBit( TIM3, TIM_IT_Update);    // Clear the Update bit
	TIM_ITConfig( TIM3, TIM_IT_Update, ENABLE);	    // Enable an interrupt for TIM3 when it Updates

	// NVIC config
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;             // TIM3 is the interrupt in question
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;   // Lower priority than TIM2_IRQn
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// TIM4 will run at the same speed as TIM2, but offset by half a period. This way
	// the DAC will update when the ADC is not sampling and vice versa. Just so you know
	// TIM2 will first update and half a clock later TIM4 will update for the first time.

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	memset( (void*)&TIMInit, 0, sizeof(TIM_TimeBaseInitTypeDef) );
	TIMInit.TIM_Prescaler       = 1;
	TIMInit.TIM_CounterMode     = TIM_CounterMode_Up;
	TIMInit.TIM_Period	    = ADC_PERIOD >> 1;   // Wait for half a period before updating (and halting!)
	TIMInit.TIM_ClockDivision   = TIM_CKD_DIV1;

	// Init
	TIM_TimeBaseInit(TIM4, &TIMInit );
	TIM_SelectOnePulseMode(TIM4, TIM_OPMode_Single);	// Only one pulse (TRGO2 will restart the timer)
	TIM_SelectSlaveMode(TIM4, TIM_SlaveMode_Trigger);	// TIM3 will be a slave to TIM2
	TIM_SelectInputTrigger(TIM4, TIM_TS_ITR1);	    // Set the input trigger to TRGO2 = ITR1 (ITR = TRGO - 1 ;-)
   
	TIM_SelectOutputTrigger(TIM4, TIM_TRGOSource_Update); // TRGO on update

}

void enable_clock(){
	TIM_Cmd(TIM2, ENABLE);
}

function clock_TIM3_IRQHook;

void TIM3_IRQHandler(void) {
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
 	if (clock_TIM3_IRQHook != NULL){
		clock_TIM3_IRQHook();
	}
}

