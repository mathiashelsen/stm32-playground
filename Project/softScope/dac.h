#ifndef _DAC_H_
#define _DAC_H_

#include <stm32f4xx.h>

// Initialize the AWG output pin(s)
void init_analogOut();

// Initialize the DAC
void init_DAC(volatile uint16_t *samplesBuffer, int samples);

#endif
