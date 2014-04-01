/*
 * USART communication.
 *
 * Based on http://eliaselectronics.com/stm32f4-tutorials
 */
#include <misc.h>
#include <stdint.h>
#include <stdlib.h>
#include <stm32f4xx.h>
#include <stm32f4xx_usart.h>

#include "usart.h"
#include "leds.h"

volatile bool transmitting;
function USART_postTXHook;

void init_USART1(uint32_t baudrate) {

	// Enable APB2 peripheral clock for USART1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	// Enable peripheral clock for the USART pins
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	// Sets up TX and RX pins
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;              // Alternate Function
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;         // Better 25(?)
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;            // Push-Pull
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;              // Pullup
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	// Hand over pin control to usart.
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = baudrate;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;                     // 8 bit words
	USART_InitStruct.USART_StopBits = USART_StopBits_1;                          // 1 stop bit
	USART_InitStruct.USART_Parity = USART_Parity_No;                             // no parity
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // no flow control
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;                 // enable TX and RX
	USART_Init(USART1, &USART_InitStruct);

	// Enable receive interrupt
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	// Configure interrupts
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;        // Configure USART1 interrupts
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// Priority group
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;       // Subpriority inside the group
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;          // enable globally
	NVIC_Init(&NVIC_InitStructure);

	// Enable USART
	USART_Cmd(USART1, ENABLE);
}


void USART_TX(USART_TypeDef* USARTx, uint8_t *data, uint16_t N) {
	for(uint16_t i=0; i<N; i++) {
		while( !(USARTx->SR & 0x00000040) ) {
			// wait until data register is empty
		}
		USART_SendData(USARTx, data[i]);
	}
}



void USART_asyncTX(volatile uint16_t *usartBuffer, int samples) {

	while(transmitting){
		// wait for previous transmit
	}

	transmitting = true;
				
	// Start transmitting using DMA, interrupt when finished -> transmitting = 0
	DMA_InitTypeDef usartDMA = {0, };
	usartDMA.DMA_Channel            = DMA_Channel_4; // channel 4, stream 7 = USART1_TX
	usartDMA.DMA_PeripheralBaseAddr = (uint32_t) &(USART1->DR);
	usartDMA.DMA_Memory0BaseAddr    = (uint32_t) usartBuffer;
	usartDMA.DMA_DIR                = DMA_DIR_MemoryToPeripheral;
	usartDMA.DMA_BufferSize         = samples * sizeof(uint16_t);
	usartDMA.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	usartDMA.DMA_MemoryInc          = DMA_MemoryInc_Enable;
	usartDMA.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	usartDMA.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte; // for some reason HalfWord does not work
	usartDMA.DMA_Mode               = DMA_Mode_Normal;
	usartDMA.DMA_Priority           = DMA_Priority_Low;        // This DMA is shared with the ADC, which has priority ofcourse
	usartDMA.DMA_FIFOMode           = DMA_FIFOMode_Disable;
	usartDMA.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
	usartDMA.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;

	DMA_Init( DMA2_Stream7, &usartDMA );
	DMA_ClearITPendingBit( DMA2_Stream7, DMA_IT_TC);
	DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE );
	// Configure interrupts
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;  // Configure USART1 interrupts
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;// Priority group
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;       // Subpriority inside the group
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;          // enable globally
	NVIC_Init(&NVIC_InitStructure);
	DMA_Cmd( DMA2_Stream7, ENABLE );
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
}



static volatile header_t headerBuf;
static uint8_t* headerArray = (uint8_t*)(&headerBuf);
static volatile int arrayPos = 0;


void USART1_IRQHandler() {
 	// check if the USART1 receive interrupt flag was set
 	if( USART_GetITStatus(USART1, USART_IT_RXNE) ) {
		LEDOn(LED1);
 		headerArray[arrayPos] = USART1->DR;
		arrayPos++;
		if (arrayPos >= HEADER_BYTES){
			arrayPos = 0;
			incomingHeader = headerBuf; // header complete, copy to visible header
		}
		USART_ClearITPendingBit(USART1, USART_IT_RXNE); // clear receive register not empty bit
		LEDOff(LED1);
 	}

}






void DMA2_Stream7_IRQHandler() {
	//DMA_ClearITPendingBit( DMA2_Stream7, DMA_IT_TC );
	DMA2->HIFCR = (1 << 27 | 1 << 26);

	USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);
	GPIO_ResetBits(GPIOD, GPIO_Pin_14);
	DMA_Cmd( DMA2_Stream7, DISABLE );
	transmitting = false;
	if (USART_postTXHook != NULL){
		USART_postTXHook();
	}
}

