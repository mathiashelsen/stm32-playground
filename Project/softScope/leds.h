#ifndef _LEDS_H_
#define _LEDS_H_

#include <stm32f4xx.h>

#define LED1 GPIO_Pin_12  // green
#define LED2 GPIO_Pin_13  // orange 
#define LED3 GPIO_Pin_14  // red
#define LED4 GPIO_Pin_15  // blue

// Initialize GPIO for the onboard LEDS
void init_LEDs();

// Turns LED on.
void LEDOn (uint16_t led);

// Turns LED off.
void LEDOff(uint16_t led);

#endif
