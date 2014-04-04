#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "usart.h"  // for definition of "function"
#include <string.h>


// Initialize the ADC/DAC clocks. They will sample every ADCPeriod cycles (42=1Msample/s).
// Generate TIM3 interrupt every IRPeriod (then clock_TIM3_IRQHook will be called).
void init_clock(int ADCPeriod, int IRPeriod);

// Starts the ADC/DAC clocks, to be called after each init_clock();
void enable_clock();

// Called at the end of TIM3_IRQHandler.
function clock_TIM3_IRQHook;

#endif
