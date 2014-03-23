#include "stm32f4xx.h"
#include "misc.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define BUFFER_SIZE 128

volatile uint16_t	DMA_BUFFER[BUFFER_SIZE];

// These should be called in the following order:
void init_DMA(void);
void init_DAC(void);
void init_TIM2(void);

int main(void)
{
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 

    for(uint16_t i = 0; i < BUFFER_SIZE; i++)
    {
	DMA_BUFFER[i] = (i << 2);
    }

    init_TIM2();
    init_DAC();
    init_DMA();

    TIM_Cmd(TIM2, ENABLE);
    DAC_Cmd( DAC_Channel_1, ENABLE );
    DAC_DMACmd( DAC_Channel_1, ENABLE ); 
    DMA_Cmd( DMA1_Stream5, ENABLE );
    while(1)
    {
	
    }

}

void init_DMA(void)
{
    // Enable the clock to the DMA
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

    // Configure the DMA stream for memory -> DAC
    DMA_InitTypeDef DMAInit = {0, };
    DMAInit.DMA_Channel	    = DMA_Channel_7; // DMA channel 7 stream 5 is mapped to DAC1
    DMAInit.DMA_PeripheralBaseAddr  = (uint32_t) 0x40007408; // Magic!
    DMAInit.DMA_Memory0BaseAddr	    = (uint32_t) DMA_BUFFER; // Copy data from the buffer
    DMAInit.DMA_DIR	    = DMA_DIR_MemoryToPeripheral;
    DMAInit.DMA_BufferSize  = BUFFER_SIZE;
    DMAInit.DMA_PeripheralInc	    = DMA_PeripheralInc_Disable; // Do not increase the periph pointer
    DMAInit.DMA_MemoryInc   = DMA_MemoryInc_Enable; // But do increase the memory pointer
    DMAInit.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_HalfWord; //16 bits only please
    DMAInit.DMA_MemoryDataSize	    = DMA_MemoryDataSize_HalfWord;
    DMAInit.DMA_Mode	    = DMA_Mode_Circular; // Wrap around and keep playing a shanty tune
    DMAInit.DMA_Priority    = DMA_Priority_VeryHigh;
    DMAInit.DMA_FIFOMode    = DMA_FIFOMode_Disable; // No FIFO, direct write will be sufficiently fast
    DMAInit.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMAInit.DMA_PeripheralBurst	    = DMA_PeripheralBurst_Single;
    
    // Initialize the DMA
    DMA_Init( DMA1_Stream5, &DMAInit );
}

void init_DAC(void)
{
    // Now to output to some pin ofcourse, PA4 for example (routed to audio PA)
    //  Enable clock to GPIOA
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // Init the pins
    GPIO_InitTypeDef GPIOInit = {0, };
    GPIOInit.GPIO_Pin	= GPIO_Pin_4; // PA4
    GPIOInit.GPIO_Mode	= GPIO_Mode_AN; // Analog function
    
    GPIO_Init(GPIOA, &GPIOInit);

    // Enable the clock to the DAC
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
    
    // Configure the DAC with DMA and Timer trigger
    DAC_InitTypeDef DACInit = {0, };
    DACInit.DAC_Trigger	    = DAC_Trigger_T2_TRGO; // Trigger of timer 2
    DACInit.DAC_WaveGeneration	= DAC_WaveGeneration_None; // No noise or triangle
    DACInit.DAC_OutputBuffer	= DAC_OutputBuffer_Disable;

    // Init DAC1
    DAC_Init( DAC_Channel_1, &DACInit );
}

void init_TIM2(void)
{
    // Enable the clock to the timer 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // APB1 clock: 168MHz/4 = 42 MHz

    TIM_TimeBaseInitTypeDef TIMInit = {0, }; 
    TIMInit.TIM_Prescaler   = 1;
    TIMInit.TIM_CounterMode = TIM_CounterMode_Up;
    TIMInit.TIM_Period	    = 420; //Timer overflows at 1MHz rate
    TIMInit.TIM_ClockDivision	= TIM_CKD_DIV1;

    // Init but DON'T enable
    TIM_TimeBaseInit(TIM2, &TIMInit );
   
    // Generate a trigger signal on update 
    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
}
