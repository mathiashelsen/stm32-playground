#include "dac.h"

void init_DACs(void)
{
    // Now to output to some pin ofcourse, PA4 for example (routed to audio PA)
    //  Enable clock to GPIOA
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // Init the pins
    GPIO_InitTypeDef GPIOInit = {0, };
    GPIOInit.GPIO_Pin	= GPIO_Pin_4 | GPIO_Pin_5; // PA4
    GPIOInit.GPIO_Mode	= GPIO_Mode_AN; // Analog function
    GPIOInit.GPIO_PuPd	= GPIO_PuPd_NOPULL;
    
    GPIO_Init(GPIOA, &GPIOInit);

    // Enable the clock to the DAC
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
    DAC_InitTypeDef dacInit;
    memset( (void *)&dacInit, 0, sizeof(DAC_InitTypeDef) );
    dacInit.DAC_Trigger = DAC_Trigger_None;
    dacInit.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
    
    DAC_Init(DAC_Channel_1, &dacInit);
    DAC_Cmd(DAC_Channel_1, ENABLE);

    DAC_Init(DAC_Channel_2, &dacInit);
    DAC_Cmd(DAC_Channel_2, ENABLE);
}
