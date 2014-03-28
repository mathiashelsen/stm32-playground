#include "stm32f4xx.h"
#include "misc.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

volatile uint16_t *samplesBuffer;
volatile uint16_t *usartBuffer;
volatile uint32_t triggerFrame; // A number between 0..3 that indicates
				// in which frame we need to look for a trigger
volatile uint16_t triggerLevel;

#define ADC_PERIOD  4200 // Divider for the ADC clock
#define SAMPLES	    1024 // Number of samples for each acquisition

void init_clock(void);
void init_ADC(void);

int main(void)
{
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 

    samplesBuffer = malloc(sizeof(uint16_t)*SAMPLES*4);

    init_clock();
    init_ADC();

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    GPIO_InitTypeDef gpio = {0, };
    gpio.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_25MHz;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &gpio);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    memset((void*) &gpio, 0, sizeof(GPIO_InitTypeDef));
    gpio.GPIO_Pin = GPIO_Pin_1;
    gpio.GPIO_Mode = GPIO_Mode_AN;
    GPIO_Init(GPIOA, &gpio);
    
    TIM_Cmd(TIM2, ENABLE);
    while(1)
    {
	
    }

}

void TIM3_IRQHandler(void)
{
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    GPIO_ToggleBits( GPIOD, GPIO_Pin_12 );
   
    /*
     * TRIGGER DETECTION
     */ 
    uint16_t *triggerPoint = NULL;
    uint16_t *sptr = samplesBuffer + 1024*triggerFrame;
    uint16_t x0, x1, x2, x3, x4;
    uint32_t N = SAMPLES>>2;
    do
    {
	x0 = *sptr++;
	x1 = *sptr++;
	x2 = *sptr++;
	x3 = *sptr++;
	x4 = *sptr; // x0 = x4 in the next iteration
	if(x0 > triggerLevel)
	{
	    if( x1 > x0 )
	    {
		triggerPoint = sptr-4;
		N = 0;
	    }
	}
	else if( (N != 0) && (x1 > triggerLevel))
	{
	    if( x2 > x1 )
	    {
		triggerPoint = sptr-3;
		N = 0;
	    }
	}
	else if((N != 0) && (x2 > triggerLevel) )
	{
	    if( x3 > x2 )
	    {
		triggerPoint = sptr-2;
		N = 0;
	    }
	}
	else if((N != 0) && (x2 > triggerLevel) )
	{
	    if( x4 > x3 )
	    {
		triggerPoint = sptr-1;
		N = 0;
	    }

	}
    }while(N--);
    // Update the triggerFrame
    triggerFrame++;
    if( triggerFrame == 4 )
    {
	triggerFrame = 0;
    }

    /*
     * DATA TRANSFER (IF USART IDLE)
     */ 
    if(triggerPoint && !transmitting)
    {
	
	// Start transmitting using DMA
	DMA_InitTypeDef;
	void DMA_Init(DMA_Stream_TypeDef* DMAy_Streamx, DMA_InitTypeDef* DMA_InitStruct);
	void DMA_Cmd(DMA_Stream_TypeDef* DMAy_Streamx, FunctionalState NewState);
    }
}

void init_clock(void)
{
    // Enable the clock to the timer 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // APB1 clock: 168MHz/4 = 42 MHz

    TIM_TimeBaseInitTypeDef TIMInit = {0, }; 
    TIMInit.TIM_Prescaler   = 1;
    TIMInit.TIM_CounterMode = TIM_CounterMode_Up;
    TIMInit.TIM_Period	    = ADC_PERIOD;
    TIMInit.TIM_ClockDivision	= TIM_CKD_DIV1;

    // Init but DON'T enable
    TIM_TimeBaseInit(TIM2, &TIMInit );

    // Trigger output on update
    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);


    // Enable clock to TIM3
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIMInit.TIM_Period		    = SAMPLES-1; // Wait for 1024 samples to be aqcuired

    // Init
    TIM_TimeBaseInit(TIM3, &TIMInit );
    //TIM_SelectOnePulseMode(TIM3, TIM_OPMode_Single);	// Only one pulse (TRGO2 will restart the timer)
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Trigger);	// TIM3 will be a slave to TIM2
  //  TIM_SelectInputTrigger(TIM3, TIM_TS_ITR1);		// Set the input trigger to TRGO2 = ITR1 (ITR = TRGO - 1 ;-)
    TIM_ITRxExternalClockConfig(TIM3, TIM_TS_ITR1);
   
    TIM_Cmd(TIM3, ENABLE);
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
    DMAInit.DMA_Memory0BaseAddr	    = (uint32_t) samplesBuffer; // Copy data from the buffer
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

