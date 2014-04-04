#include <stm32f4xx.h>
#include <stdlib.h>

#include "clock.h"


void enable_clock(){
	TIM_Cmd(TIM2, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
	TIM_Cmd(TIM4, ENABLE);
}

void disable_clock(){
	TIM_Cmd(TIM2, DISABLE);
	TIM_Cmd(TIM3, DISABLE);
	TIM_Cmd(TIM4, DISABLE);
}

void init_clock(int ADC_PERIOD, int IR_PERIOD) {

	disable_clock();
	
	// TIM2 controls the ADC sample rate
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);      // Enable clock to peripheral, APB1 clock: 168MHz/4 = 42 MHz
	TIM_TimeBaseInitTypeDef TIM2Init = {0, };
	TIM2Init.TIM_Prescaler     = 1;
	TIM2Init.TIM_CounterMode   = TIM_CounterMode_Up;
	TIM2Init.TIM_Period	       = ADC_PERIOD - 1;
	TIM2Init.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM2, &TIM2Init );                       // Init but DON'T enable
	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);     // Trigger output on update

	// TIM3 runs at IR_PERIOD, generates interrupts so we know where the ADC is.
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);      // Enable clock to peripheral

	TIM_TimeBaseInitTypeDef TIM3Init = {0, };
	TIM2Init.TIM_Prescaler     = 1;
	TIM2Init.TIM_CounterMode   = TIM_CounterMode_Up;
	TIM2Init.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM3Init.TIM_Period        = IR_PERIOD - 1; 
	TIM_TimeBaseInit(TIM3, &TIM3Init );
	TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Trigger);	      // TIM3 will be a slave to TIM2
	TIM_ITRxExternalClockConfig(TIM3, TIM_TS_ITR1);
	TIM_ClearITPendingBit( TIM3, TIM_IT_Update);              // Clear the Update bit
	TIM_ITConfig( TIM3, TIM_IT_Update, ENABLE);	              // Enable an interrupt for TIM3 when it Updates

	// NVIC config
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;           // TIM3 is the interrupt in question
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4; // Lower priority than TIM2_IRQn
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// TIM4 will run at the same speed as TIM2, but offset by half a period. This way
	// the DAC will update when the ADC is not sampling and vice versa. Just so you know
	// TIM2 will first update and half a clock later TIM4 will update for the first time.
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);      // TODO(m): review my change TIM3->TIM4
	TIM_TimeBaseInitTypeDef TIM4Init = {0, };
	TIM4Init.TIM_Prescaler       = 1;
	TIM4Init.TIM_CounterMode     = TIM_CounterMode_Up;
	TIM4Init.TIM_Period          = (ADC_PERIOD-1) >> 1;       // Wait for half a period before updating (and halting!)
	TIM4Init.TIM_ClockDivision   = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM4, &TIM4Init );
	TIM_SelectOnePulseMode(TIM4, TIM_OPMode_Single);	      // Only one pulse (TRGO2 will restart the timer)
	TIM_SelectSlaveMode(TIM4, TIM_SlaveMode_Trigger);	      // TIM3 will be a slave to TIM2
	TIM_SelectInputTrigger(TIM4, TIM_TS_ITR1);	              // Set the input trigger to TRGO2 = ITR1 (ITR = TRGO - 1 ;-)
	TIM_SelectOutputTrigger(TIM4, TIM_TRGOSource_Update);     // TRGO on update
}

function clock_TIM3_IRQHook;

void TIM3_IRQHandler(void) {
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
 	if (clock_TIM3_IRQHook != NULL){
		clock_TIM3_IRQHook();
	}
}

