#include <stm32f4xx.h>
#include <stdint.h>
#include <string.h>

#include "adc.h"

void init_analogIn(){
	GPIO_InitTypeDef gpio = {0, };
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	memset((void*) &gpio, 0, sizeof(GPIO_InitTypeDef));
	gpio.GPIO_Pin = GPIO_Pin_1;
	gpio.GPIO_Mode = GPIO_Mode_AN;
	GPIO_Init(GPIOA, &gpio);
}

void init_ADC(volatile uint16_t *samplesBuffer, int SAMPLES) {

	// Init the DMA for transferring data from the ADC
	// Enable the clock to the DMA
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	// Configure the DMA stream for ADC -> memory
	DMA_InitTypeDef DMAInit = {0, };
	DMAInit.DMA_Channel            = DMA_Channel_0;                   // DMA channel 0 stream 0 is mapped to ADC1
	DMAInit.DMA_PeripheralBaseAddr = (uint32_t) 0x4001204c;
	DMAInit.DMA_Memory0BaseAddr	   = (uint32_t) samplesBuffer;        // Copy data from the buffer
	DMAInit.DMA_DIR	               = DMA_DIR_PeripheralToMemory;
	DMAInit.DMA_BufferSize         = SAMPLES*4;
	DMAInit.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;       // Do not increase the periph pointer
	DMAInit.DMA_MemoryInc          = DMA_MemoryInc_Enable;            // But do increase the memory pointer
	DMAInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //16 bits only please
	DMAInit.DMA_MemoryDataSize	   = DMA_MemoryDataSize_HalfWord;
	DMAInit.DMA_Mode               = DMA_Mode_Circular;               // Wrap around and keep playing a shanty tune
	DMAInit.DMA_Priority           = DMA_Priority_VeryHigh;
	DMAInit.DMA_FIFOMode           = DMA_FIFOMode_Disable;            // No FIFO, direct write will be sufficiently fast
	DMAInit.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
	DMAInit.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;

	// Initialize the DMA
	DMA_Init( DMA2_Stream0, &DMAInit );
	DMA_Cmd( DMA2_Stream0 , ENABLE );

	//DMA_ADC1 = DMA2_Stream0;

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
	ADC_DMACmd( ADC1, ENABLE ); // Enable generating DMA requests
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

	// Configure the channel from which to sample
	ADC_RegularChannelConfig( ADC1, ADC_Channel_1, 1, ADC_SampleTime_3Cycles);

	// NVIC config
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	ADC_ITConfig(ADC1, ADC_IT_OVR, ENABLE);
	ADC_ClearFlag(ADC1, ADC_FLAG_OVR);
	ADC_ClearITPendingBit(ADC1 , ADC_IT_OVR);

	ADC_Cmd( ADC1, ENABLE );

	ADC_SoftwareStartConv(ADC1);
}

void ADC_IRQHandler(void) {
	ADC_ClearITPendingBit(ADC1 , ADC_IT_OVR);
	ADC_ClearFlag(ADC1, ADC_FLAG_OVR);
	DMA_Cmd( DMA2_Stream0, ENABLE );
}

