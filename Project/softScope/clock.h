#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "usart.h"  // for definition of "function"
#include <string.h>

// Starts the ADC clock
void enable_clock();

// Initialize the ADC clock
void init_clock(int ADC_PERIOD, int SAMPLES);

// Called at the end of TIM3_IRQHandler.
function clock_TIM3_IRQHook;

#endif
