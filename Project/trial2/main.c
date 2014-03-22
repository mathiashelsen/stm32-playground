#include "stm32f4xx.h"
#include "misc.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

/*
 * In this example TIM2 and TIM3 are both generating triggers for
 * e.g. DMA, DAC, ADC, ... TIM2 is configured in the usual fashion,
 * but TIM3 is configured as single pulse timer, which updates after
 * half of the period of TIM2. Because of this, update events of TIM2
 * and TIM3 are shifted by 180degrees. Ofcourse, you can vary the delay
 * to generate any phase shift you would like.
 * In comparison to multiple OC's on a single timer this has the advantage it can be used 
 * to trigger multiple internal periphs.
 */

#define TIM_PERIOD 100

// These should be called in the following order:
void init_TIM2(void); // TIM2 running at 1MHz
void init_TIM3(void); // TIM3, running at the same frequency, but 180degrees later

int main(void)
{
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 

    init_TIM2();
    init_TIM3();
    
    // Configure TIM3 OC1 on an output pin
    //  Enable clock to GPIOA
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    // Init the pins
    GPIO_InitTypeDef GPIOInit = {0, };
    GPIOInit.GPIO_Pin	= GPIO_Pin_6 | GPIO_Pin_7;
    GPIOInit.GPIO_Mode	= GPIO_Mode_OUT; // Analog function
    GPIOInit.GPIO_OType	= GPIO_OType_PP;
    GPIOInit.GPIO_Speed	= GPIO_Speed_25MHz;
    
    GPIO_Init(GPIOC, &GPIOInit);

    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
    while(1)
    {
	
    }

}

// Toggle PC6 on TIM2 Update
void TIM2_IRQHandler(void)
{
    if( TIM_GetITStatus( TIM2, TIM_IT_Update) != RESET )
    {
	TIM_ClearITPendingBit( TIM2, TIM_IT_Update );
	GPIO_ToggleBits( GPIOC, GPIO_Pin_6 );
    }
}

// and toggle PC7 on TIM3 update
void TIM3_IRQHandler(void)
{
    if( TIM_GetITStatus( TIM3, TIM_IT_Update) != RESET )
    {
	TIM_ClearITPendingBit( TIM3, TIM_IT_Update );
	GPIO_ToggleBits( GPIOC, GPIO_Pin_7 );
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

void init_TIM3(void)
{
    /*
     * TIM3 will be running at the same frequency as TIM2 and
     * will be restarted by the latter. It will only run for 21
     * clock cycles after which it will halt and generate a UEV
     */
    // Enable clock to TIM3
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInitTypeDef TIMInit = {0, }; 
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

    // To test the config, we will be calling interrupts
    TIM_ITConfig( TIM3, TIM_IT_Update, ENABLE);	    // Enable an interrupt for TIM3 when it Updates
    TIM_ClearITPendingBit( TIM3, TIM_IT_Update);    // Clear the Update bit

    // NVIC config
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; // TIM3 is the interrupt in question
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;	// Lower priority than TIM2_IRQn
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
