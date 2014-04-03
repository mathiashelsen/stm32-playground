#include "dac.h"

void init_analogOut()
{
    // Now to output to some pin ofcourse, PA4 for example (routed to audio PA)
    //  Enable clock to GPIOA
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // Init the pins
    GPIO_InitTypeDef GPIOInit = {0, };
    GPIOInit.GPIO_Pin	= GPIO_Pin_4; // PA4
    GPIOInit.GPIO_Mode	= GPIO_Mode_AN; // Analog function
    
    GPIO_Init(GPIOA, &GPIOInit);
}

void init_DAC(volatile uint16_t *samplesBuffer, int samples)
{
    // Enable the clock to the DAC
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
    
    // Configure the DAC with DMA and Timer trigger
    DAC_InitTypeDef DACInit = {0, };
    DACInit.DAC_Trigger	    = DAC_Trigger_T4_TRGO; // Trigger of timer 4
    DACInit.DAC_WaveGeneration	= DAC_WaveGeneration_None; // No noise or triangle
    DACInit.DAC_OutputBuffer	= DAC_OutputBuffer_Disable;

    // Init DAC1
    DAC_Init( DAC_Channel_1, &DACInit );

    // Enable the clock to the DMA
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

    // Configure the DMA stream for memory -> DAC
    DMA_InitTypeDef DMAInit = {0, };
    DMAInit.DMA_Channel	    = DMA_Channel_7; // DMA channel 7 stream 5 is mapped to DAC1
    DMAInit.DMA_PeripheralBaseAddr  = (uint32_t) 0x40007408; // Magic!
    DMAInit.DMA_Memory0BaseAddr	    = (uint32_t) samplesBuffer; // Copy data from the buffer
    DMAInit.DMA_DIR	    = DMA_DIR_MemoryToPeripheral;
    DMAInit.DMA_BufferSize  = (uint32_t) samples;
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

    DAC_Cmd( DAC_Channel_1, ENABLE );
    DAC_DMACmd( DAC_Channel_1, ENABLE ); 
    DMA_Cmd( DMA1_Stream5, ENABLE );
}
