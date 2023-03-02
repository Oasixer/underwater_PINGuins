#ifndef FYDP_ADC_ISR_H
#define FYDP_ADC_ISR_H

#include "constants.h"

void adc_isr();
uint16_t* adc_setup();
void adc_timer_callback();
float* get_frequency_magnitudes();

#endif
