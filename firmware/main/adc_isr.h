#ifndef FYDP_ADC_ISR_H
#define FYDP_ADC_ISR_H

#include "constants.h"

void adc_isr();
void adc_setup();
void adc_timer_callback();
float* get_frequency_magnitudes();
uint16_t* get_last_reading();

#endif
