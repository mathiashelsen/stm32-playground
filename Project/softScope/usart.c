/*
 * USART communication.
 *
 * Based on http://eliaselectronics.com/stm32f4-tutorials
 */
#include <misc.h>
#include <stdint.h>
#include <stm32f4xx.h>
#include <stm32f4xx_usart.h>

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
		while( !(USARTx->SR & 0x00000040) ){
			// wait until data register is empty
		}
		USART_SendData(USARTx, data[i]);
	}
}


// this is the interrupt request handler (IRQ) for ALL USART1 interrupts
// void USART1_IRQHandler(void) {
// 
// 	// check if the USART1 receive interrupt flag was set
// 	if( USART_GetITStatus(USART1, USART_IT_RXNE) ) {
// 		uint8_t data = USART1->DR;
// 		// do something with data
// 	}
// }
