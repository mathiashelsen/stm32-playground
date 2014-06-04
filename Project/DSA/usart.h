#ifndef _USART_H
#define _USART_H

/* Copyright 2014 Mathias Helsen */

#include "stm32f4xx.h"
#include "misc.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

void init_USART(void);
void transmit(uint32_t size, volatile uint8_t *buffer);
uint32_t readWord(void);
uint16_t readHalfword(void);

#endif
