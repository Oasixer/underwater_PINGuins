#ifndef FYDP_ADC_ISR_H
#define FYDP_ADC_ISR_H

#include "constants.h"

void adc_isr();
void adc_setup();
void adc_timer_callback();
float* get_frequency_magnitudes();
float* get_largest_magnitudes();
uint16_t* get_last_reading();
uint64_t* get_fourier_counter();

#endif
