#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "usart.h"  // TODO: rm, split-off typedef function

void enable_clock();

void init_clock(int ADC_PERIOD, int SAMPLES);

function clock_TIM3_IRQHook;

#endif
