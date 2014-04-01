#ifndef _USART_H_
#define _USART_H_

#include <stdbool.h>

typedef void (*function)(void);

// Init USART1 with TX on pin PB6, RX on PB7
void init_USART1(uint32_t baudrate);

// Send N bytes of data. Spins while waiting for transfer.
void USART_TX(USART_TypeDef* USARTx, uint8_t *data, uint16_t N);

// Send usartBuffer with size SAMPLES asynchronously. Sets transmitting variable.
void USART_asyncTX(volatile uint16_t *usartBuffer, int SAMPLES);

// Whether USART_asyncTX is transmitting
volatile bool transmitting;

// Called after USART_asyncTX.
function USART_postTXHook;

// Frame data header
typedef struct{
	uint32_t magic;    // identifies start of header, 0xFFFFFFFF
	uint32_t samples;  // number of samples
} header_t;

#endif
