#include "stm32f4xx.h"
#include "misc.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

/*
 * This curve tracer will use two ADC's and two DAC's:
 * DAC1: Va/Vc/Vd, DMA supplied, timer triggered sawtooth 
 * DAC2: Vg/Vb, software set
 * ADC1: Samples the voltage between CE/DS/AC
 * ADC2: Samples the voltage over the current sensing resistor
 * Both ADCs run from TIM3, which is half a period delayed from TIM2
 * which drives DAC1. The ADCs are emptied through DMA. Data is shipped
 * in and out through the USART
 */

#define TIM_PERIOD 100

#define BUFFER_SIZE 1024
volatile uint16_t VC_OUT_BUFFER[BUFFER_SIZE]; // DAC1
volatile uint16_t VB_BUFFER; // DAC2
volatile uint16_t VC_IN_BUFFER[BUFFER_SIZE]; // ADC1
volatile uint16_t IE_IN_BUFFER[BUFFER_SIZE]; // ADC2

// These should be called in the following order:
void init_TIM2(void); // TIM2 running at 1MHz
void init_TIM3(void); // TIM3, running at the same frequency, but 180degrees later

void init_DMA1(void); //DMA1 will be filling DAC1
void init_DMA2(void); //DMA2 will be emptying ADC1 and ADC2

void init_ADC_VC(void); // Init the ADC that samples the collector-emitter voltage
void init_ADC_IE(void); // Init the ADC that samples the emitter current

void init_USART(void); // Unimplemented

int main(void)
{
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 

    init_TIM2();
    init_TIM3();
    
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
    //TIM_Cmd(TIM3, ENABLE);
    while(1)
    {
	
    }

}

void DMA2_Stream0_IRQHandler(void)
{
    // Disable TIM2 and TIM3 and clear both counters

    // Copy the data and send out through the USART

    // Update (if necessary) VBE/VG
	// Reinit DMA's
	// Restart timers 
    
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
}

//DMA1 will supplying DAC1, this will run empty at a certain point.
void init_DMA1(void)
{
    // Firstly, init DMA for DAC1
    // Enable the clock to the DMA
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

    // Configure the DMA stream for memory -> DAC
    DMA_InitTypeDef DMAInit = {0, };
    DMAInit.DMA_Channel	    = DMA_Channel_7; // DMA channel 7 stream 5 is mapped to DAC1
    DMAInit.DMA_PeripheralBaseAddr  = (uint32_t) 0x40007408; // Magic!
    DMAInit.DMA_Memory0BaseAddr	    = (uint32_t) VC_OUT_BUFFER; // Copy data from the buffer
    DMAInit.DMA_DIR	    = DMA_DIR_MemoryToPeripheral;
    DMAInit.DMA_BufferSize  = BUFFER_SIZE;
    DMAInit.DMA_PeripheralInc	    = DMA_PeripheralInc_Disable; // Do not increase the periph pointer
    DMAInit.DMA_MemoryInc   = DMA_MemoryInc_Enable; // But do increase the memory pointer
    DMAInit.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_HalfWord; //16 bits only please
    DMAInit.DMA_MemoryDataSize	    = DMA_MemoryDataSize_HalfWord;
    DMAInit.DMA_Mode	    = DMA_Mode_Normal; // Wrap around and keep playing a shanty tune
    DMAInit.DMA_Priority    = DMA_Priority_VeryHigh;
    DMAInit.DMA_FIFOMode    = DMA_FIFOMode_Disable; // No FIFO, direct write will be sufficiently fast
    DMAInit.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMAInit.DMA_PeripheralBurst	    = DMA_PeripheralBurst_Single;
    
    // Initialize the DMA
    DMA_Init( DMA1_Stream5, &DMAInit );
    // and enable
    DMA_Cmd( DMA1_Stream5, ENABLE );
}

void init_DMA2(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    // DMA for ADC1 (stream 0, channel 0, very high priority)
    DMA_InitTypeDef DMAInit = {0, };
    DMAInit.DMA_Channel	    = DMA_Channel_0; // DMA channel 0 stream 0 is mapped to ADC1/VCE
    DMAInit.DMA_PeripheralBaseAddr  = (uint32_t) &(ADC1->DR);
    DMAInit.DMA_Memory0BaseAddr	    = (uint32_t) VC_IN_BUFFER;
    DMAInit.DMA_DIR	    = DMA_DIR_PeripheralToMemory;
    DMAInit.DMA_BufferSize  = BUFFER_SIZE;
    DMAInit.DMA_PeripheralInc	    = DMA_PeripheralInc_Disable;
    DMAInit.DMA_MemoryInc   = DMA_MemoryInc_Enable;
    DMAInit.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_HalfWord;
    DMAInit.DMA_MemoryDataSize	    = DMA_MemoryDataSize_HalfWord;
    DMAInit.DMA_Mode	    = DMA_Mode_Normal; 
    DMAInit.DMA_Priority    = DMA_Priority_VeryHigh;
    DMAInit.DMA_FIFOMode    = DMA_FIFOMode_Disable;
    DMAInit.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMAInit.DMA_PeripheralBurst	    = DMA_PeripheralBurst_Single;
    DMA_Init( DMA2_Stream0, &DMAInit );
    DMA_Cmd( DMA2_Stream0, ENABLE );

    // DMA for ADC2 (stream 2, channel 1, high priority)
    DMAInit.DMA_Channel	    = DMA_Channel_1; // DMA channel 0 stream 0 is mapped to ADC1/VCE
    DMAInit.DMA_PeripheralBaseAddr  = (uint32_t) &(ADC2->DR);
    DMAInit.DMA_Memory0BaseAddr	    = (uint32_t) IE_IN_BUFFER;
    DMAInit.DMA_DIR	    = DMA_DIR_PeripheralToMemory;
    DMAInit.DMA_BufferSize  = BUFFER_SIZE;
    DMAInit.DMA_PeripheralInc	    = DMA_PeripheralInc_Disable;
    DMAInit.DMA_MemoryInc   = DMA_MemoryInc_Enable;
    DMAInit.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_HalfWord;
    DMAInit.DMA_MemoryDataSize	    = DMA_MemoryDataSize_HalfWord;
    DMAInit.DMA_Mode	    = DMA_Mode_Normal; 
    DMAInit.DMA_Priority    = DMA_Priority_High; // A bit lower than stream 0
    DMAInit.DMA_FIFOMode    = DMA_FIFOMode_Disable;
    DMAInit.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMAInit.DMA_PeripheralBurst	    = DMA_PeripheralBurst_Single;
    DMA_Init( DMA2_Stream2, &DMAInit );
    DMA_Cmd( DMA2_Stream2, ENABLE );

    // When all the samples have been collected, we throw an interrupt to stop this scan
    DMA_ITConfig( DMA2_Stream0, DMA_IT_TC, ENABLE); // Enable the DMA to throw an iterrupt if the buffer is full
    DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TC); // Transfer Complete sets the interrupt
     
    NVIC_InitTypeDef NVICInit = {0, };
    NVICInit.NVIC_IRQChannel = DMA2_Stream0_IRQn;
    NVICInit.NVIC_IRQChannelPreemptionPriority = 0;
    NVICInit.NVIC_IRQChannelSubPriority = 0;
    NVICInit.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVICInit);
}
