#include "timer.h"

void init_Timers(void)
{
    // Enable the clock to the timer 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // APB1 clock: 168MHz/4 = 42 MHz

    TIM_TimeBaseInitTypeDef TIMInit;
    memset( (void *) &TIMInit, 0, sizeof(TIM_TimeBaseInitTypeDef) );
    TIMInit.TIM_Prescaler   = 1;
    TIMInit.TIM_CounterMode = TIM_CounterMode_Up;
    TIMInit.TIM_Period	    = TIM_PERIOD - 1; // running at 10kHz
    TIMInit.TIM_ClockDivision	= TIM_CKD_DIV1;

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

    /*
     * TIM3 will be running at the same frequency as TIM2 and
     * will be restarted by the latter. It will only run for 21
     * clock cycles after which it will halt and generate a UEV
     */
    // Enable clock to TIM3
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    memset( (void *) &TIMInit, 0, sizeof(TIM_TimeBaseInitTypeDef) );
    TIMInit.TIM_Prescaler	    = 1;
    TIMInit.TIM_CounterMode	    = TIM_CounterMode_Up;
    TIMInit.TIM_Period		    = TIM_PERIOD >> 1;   // Wait for half a period before updating (and halting!)
    TIMInit.TIM_ClockDivision	    = TIM_CKD_DIV1;

    // Init
    TIM_TimeBaseInit(TIM3, &TIMInit );
    TIM_SelectOnePulseMode(TIM3, TIM_OPMode_Single);	// Only one pulse (TRGO2 will restart the timer)
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Trigger);	// TIM3 will be a slave to TIM2
    TIM_SelectInputTrigger(TIM3, TIM_TS_ITR1);		// Set the input trigger to TRGO2 = ITR1 (ITR = TRGO - 1 ;-)
   
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update); // TRGO on update
}
