#ifndef _ADC_H_
#define _ADC_H_

// Initialize the scope's analog input pin
void init_analogIn();

// Initialize the ADC to write to samplesBuffer with size SAMPLES
void init_ADC(volatile uint16_t *samplesBuffer, int SAMPLES);

#endif
