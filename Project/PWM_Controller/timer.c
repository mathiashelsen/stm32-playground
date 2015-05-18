#include "timer.h"

void init_Timers(void)
{
    // Enable the clock to the timer 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // APB1 clock: 168MHz/4 = 42 MHz

    TIM_TimeBaseInitTypeDef TIMInit;
    memset( (void *) &TIMInit, 0, sizeof(TIM_TimeBaseInitTypeDef) );
    TIMInit.TIM_Prescaler   = 1;
    TIMInit.TIM_CounterMode = TIM_CounterMode_Up;
    TIMInit.TIM_Period	    = 4199; // running at 10kHz
    TIMInit.TIM_ClockDivision	= TIM_CKD_DIV1;

    // Init but DON'T enable
    TIM_TimeBaseInit(TIM2, &TIMInit );

    TIM_OCInitTypeDef OCInit;
    OCInit.TIM_OCMode = TIM_OCMode_PWM2;
    OCInit.TIM_OutputState = TIM_OutputState_Enable;
    OCInit.TIM_OCPolarity = TIM_OCPolarity_Low;
    OCInit.TIM_Pulse = 420;
    TIM_OC3Init(TIM2, &OCInit);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_TIM2);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    TIM_Cmd(TIM2, ENABLE);
}
