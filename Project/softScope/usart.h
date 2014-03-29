#ifndef _USART_H_
#define _USART_H_

// Init USART1 with TX on pin PB6, RX on PB7
void init_USART1(uint32_t baudrate);

// Send N bytes of data. Spins while waiting for transfer.
void USART_TX(USART_TypeDef* USARTx, uint8_t *data, uint16_t N);

void USART_asyncTX(volatile uint16_t *usartBuffer, int SAMPLES);

#endif
