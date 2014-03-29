#include "stm32f4xx.h"
#include <misc.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "adc.h"
#include "clock.h"
#include "usart.h"
#include "utils.h"

#define STATE_IDLE	   ((int32_t)  0)
#define STATE_PROCESS  ((int32_t)  1)
#define STATE_OVERFLOW ((int32_t) -1)

volatile uint16_t *samplesBuffer; // The samples buffer is divided into 4 frames
volatile uint16_t *usartBuffer;   // The usart buffer holds a status HW and 1 frame
volatile uint32_t triggerFrame;   // A number between 0..3 that indicates  in which frame we need to look for a trigger
volatile uint32_t transmitting;
volatile uint16_t triggerLevel;

volatile int32_t state;

//volatile ADC_TypeDef* ADCx;     // currently unused
//volatile USART_TypeDef* USARTx; // currently unused

#define ADC_PERIOD  419 // 100kSamples
#define SAMPLES	    1024 // Number of samples for each acquisition/frame

int main(void) {
	//ADCx = ADC1;
	//USARTx = USART1;

	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

	samplesBuffer   = malloc(sizeof(uint16_t)*SAMPLES*4);
	memset((void*)samplesBuffer, 0, sizeof(uint16_t)*SAMPLES*4);
	usartBuffer	    = malloc(sizeof(uint16_t)*(SAMPLES+1)); // This should be a multiple of 32bits for easy alignment
	*usartBuffer    = 0xFFFF; // The first halfword will be all ones to signal a frame

	triggerFrame = 3;
	transmitting = 0;
	triggerLevel = (0xFFF >> 1); // trigger halfway

	init_clock(ADC_PERIOD, SAMPLES);
	init_ADC(samplesBuffer, SAMPLES);
	init_USART1(115200);
	
	// screws up web interface:
	//uint8_t hallo[] = "Hello!";
	//USART_TX( USART1, hallo, strlen((char *)hallo));

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitTypeDef gpio = {0, };
	gpio.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_Speed = GPIO_Speed_25MHz;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &gpio);
	GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 );

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	memset((void*) &gpio, 0, sizeof(GPIO_InitTypeDef));
	gpio.GPIO_Pin = GPIO_Pin_1;
	gpio.GPIO_Mode = GPIO_Mode_AN;
	GPIO_Init(GPIOA, &gpio);

	state = STATE_IDLE;

	TIM_Cmd(TIM2, ENABLE);
	while(1) {
		if( state == STATE_PROCESS ) {
			GPIO_SetBits(GPIOD, GPIO_Pin_13);
			/*
			* TRIGGER DETECTION
			* The while loop has been unrolled four times, to avoid unnecessary overhead
			* and the last four samples have been cut-off from the loop to account for possible
			* pointer wrapping into the start of the circular ADC buffer.
			*/
			uint16_t *triggerPoint = NULL;
			uint16_t *sptr = (uint16_t *) (samplesBuffer + 1024*triggerFrame);
			uint16_t x0, x1, x2, x3, x4;
			uint32_t N = (SAMPLES>>2)-2;

			x0 = *sptr++;
			x1 = *sptr++;
			x2 = *sptr++;
			x3 = *sptr++;
			x4 = *sptr; // x0 = x4 in the next iteration
			do {
				if(x0 > triggerLevel) {
					if( x1 > x0 ) {
						triggerPoint = sptr-4;
						N = 0;
					}
				} else if( (N != 0) && (x1 > triggerLevel)) {
					if( x2 > x1 ) {
						triggerPoint = sptr-3;
						N = 0;
					}
				} else if((N != 0) && (x2 > triggerLevel) ) {
					if( x3 > x2 ) {
						triggerPoint = sptr-2;
						N = 0;
					}
				} else if((N != 0) && (x3 > triggerLevel) ) {
					if( x4 > x3 ) {
						triggerPoint = sptr-1;
						N = 0;
					}
				}

				// Prepare the data for the next iteration
				x0 = *sptr++;
				x1 = *sptr++;
				x2 = *sptr++;
				x3 = *sptr++;
				x4 = *sptr; // x0 = x4 in the next iteration
			} while(N--);
			// Process last 4 samples manually
			// not yet implemented

			// Update the triggerFrame
			triggerFrame++;
			if( triggerFrame == 4 ) {
				triggerFrame = 0;
			}

			/*
			* DATA TRANSFER (IF USART IDLE)
			*/
			if(triggerPoint && !transmitting) {
				// Copy the data, using memcpy for speed reasons
				if(triggerFrame > 0) {
					// Data was from frame 0..2
					memcpy16((uint16_t*)(usartBuffer+1), (uint16_t*)triggerPoint, 1024*2);
				} else {
					// This is the number of samples till we wrap to the first frame
					int32_t samples = (int32_t)(samplesBuffer + 4*1024 - 1 - triggerPoint);
					// A block needs to be copied from the last frame
					memcpy16((uint16_t*)(usartBuffer+1), (uint16_t*)triggerPoint, samples*2);
					// and a part from the first frame
					memcpy16((uint16_t*)(usartBuffer+1+samples), (uint16_t*)samplesBuffer, (1024-samples)*2);
				}

				GPIO_SetBits(GPIOD, GPIO_Pin_14);

				transmitting = 1;
				// Start transmitting using DMA, interrupt when finished -> transmitting = 0
				DMA_InitTypeDef usartDMA = {0, };
				usartDMA.DMA_Channel = DMA_Channel_4; // channel 4, stream 7 = USART1_TX
				usartDMA.DMA_PeripheralBaseAddr = (uint32_t) &(USART1->DR);
				usartDMA.DMA_Memory0BaseAddr = (uint32_t) usartBuffer;
				usartDMA.DMA_DIR = DMA_DIR_MemoryToPeripheral;
				usartDMA.DMA_BufferSize = SAMPLES + 1;
				usartDMA.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
				usartDMA.DMA_MemoryInc = DMA_MemoryInc_Enable;
				usartDMA.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
				usartDMA.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
				usartDMA.DMA_Mode = DMA_Mode_Normal;
				usartDMA.DMA_Priority = DMA_Priority_Low; // This DMA is shared with the ADC, which has priority ofcourse
				usartDMA.DMA_FIFOMode = DMA_FIFOMode_Disable;
				usartDMA.DMA_MemoryBurst = DMA_MemoryBurst_Single;
				usartDMA.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

				DMA_Init( DMA2_Stream7, &usartDMA );
				DMA_ClearITPendingBit( DMA2_Stream7, DMA_IT_TC);
				DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE );
				// Configure interrupts
				NVIC_InitTypeDef NVIC_InitStructure;
				NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;        // Configure USART1 interrupts
				NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;// Priority group
				NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;       // Subpriority inside the group
				NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;          // enable globally
				NVIC_Init(&NVIC_InitStructure);
				DMA_Cmd( DMA2_Stream7, ENABLE );
				USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
			}
			GPIO_ResetBits(GPIOD, GPIO_Pin_13);
			state = STATE_IDLE;
		} else if (state == STATE_OVERFLOW ) {
			GPIO_SetBits(GPIOD, GPIO_Pin_12);

		}
	}
}

void USART1_IRQHandler(void) {
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
}

void TIM3_IRQHandler(void) {
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

	if( state == STATE_PROCESS ) {
		state = STATE_OVERFLOW;
	} else {
		state = STATE_PROCESS;
	}
}

void DMA2_Stream7_IRQHandler(void) {
	//DMA_ClearITPendingBit( DMA2_Stream7, DMA_IT_TC );
	DMA2->HIFCR = (1 << 27 | 1 << 26);

	USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);
	GPIO_ResetBits(GPIOD, GPIO_Pin_14);
	DMA_Cmd( DMA2_Stream7, DISABLE );
	transmitting = 0;
}

void ADC_IRQHandler(void) {
	ADC_ClearITPendingBit(ADC1 , ADC_IT_OVR);
	ADC_ClearFlag(ADC1, ADC_FLAG_OVR);
	DMA_Cmd( DMA2_Stream0, ENABLE );
}

