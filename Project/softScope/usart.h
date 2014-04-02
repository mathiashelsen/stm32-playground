#ifndef _USART_H_
#define _USART_H_

#include <stdbool.h>
#include <stdint.h>

// Init USART1 with TX on pin PB6, RX on PB7
void init_USART1(uint32_t baudrate);

// Send N bytes of data. Spins while waiting for transfer.
void USART_TX(USART_TypeDef* USARTx, uint8_t *data, uint16_t N);

// Send usartBuffer with given size asynchronously. Sets transmitting variable.
void USART_asyncTX(volatile uint8_t *usartBuffer, int bytes);

// Whether USART_asyncTX is transmitting
volatile bool transmitting;

typedef void (*function)(void);

// Called after USART_asyncTX.
function USART_postTXHook;


typedef void (*byteHandler)(uint8_t);

// Called to handle bytes received on USART1.
byteHandler USART1_RXHandler; 

#endif
