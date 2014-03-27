#include "stm32f4xx.h"
#include "misc.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

volatile uint16_t *samples;

#define ADC_PERIOD 42000

// These should be called in the following order:
void init_clock(void); // TIM2 running at 1MHz
void init_ADC(void);

int main(void)
{
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 

    samples = malloc(sizeof(uint16_t)*2048);

    init_clock(); // TIM2 running at 1MHz
    init_ADC();

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    GPIO_InitTypeDef gpio = {0, };
    gpio.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_25MHz;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &gpio);
    
    TIM_Cmd(TIM2, ENABLE);
    while(1)
    {
	
    }

}

void init_clock(void)
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
}

void init_ADC(void)
{
    //Enable the clock to the ADC
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    // The things that are shared between the three ADCs
   
    ADC_CommonInitTypeDef common = {0, };
    common.ADC_Mode = ADC_Mode_Independent;
    common.ADC_Prescaler = ADC_Prescaler_Div2;
    ADC_CommonInit( &common );
  
    // The things specific to ADC1
 
    ADC_InitTypeDef adc = {0, };
    adc.ADC_Resolution = ADC_Resolution_12b;
    adc.ADC_ScanConvMode = DISABLE; // Disable scanning multiple channels
    adc.ADC_ContinuousConvMode = DISABLE; // Disable ADC free running
    adc.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
    adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO; // Trigger of TIM2
    adc.ADC_DataAlign = ADC_DataAlign_Right;
    adc.ADC_NbrOfConversion = 1;
    ADC_Init( ADC1, &adc );

    
    // Configure the channel from which to sample
    ADC_RegularChannelConfig( ADC1, ADC_Channel_1, 0, ADC_SampleTime_3Cycles);

    ADC_DMACmd( ADC1, ENABLE ); // Enable generating DMA requests
    ADC_Cmd( ADC1, ENABLE );

    // Init the DMA for transferring data from the ADC
    // Enable the clock to the DMA
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    // Configure the DMA stream for ADC -> memory
    
    DMA_InitTypeDef DMAInit = {0, };
    DMAInit.DMA_Channel	    = DMA_Channel_0; // DMA channel 0 stream 0 is mapped to ADC1
    DMAInit.DMA_PeripheralBaseAddr  = (uint32_t) ADC1->DR;
    DMAInit.DMA_Memory0BaseAddr	    = (uint32_t) samples; // Copy data from the buffer
    DMAInit.DMA_DIR	    = DMA_DIR_PeripheralToMemory;
    DMAInit.DMA_BufferSize  = 2048;
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
    
    DMA_Init( DMA2_Stream0, &DMAInit );
    DMA_Cmd( DMA2_Stream0 , ENABLE );
    
    // Now sampling from the input pin PA1
    /*
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef gpioInit = {0, };
    gpioInit.GPIO_Pin = GPIO_Pin_1;
    gpioInit.GPIO_Mode = GPIO_Mode_AN;
    GPIO_Init(GPIOA, &gpioInit);
    */
   
}

