#ifndef FYDP_FOURIER_H
#define FYDP_FOURIER_H

#include <stdint.h>
#include <math.h>
#include <stdio.h>

#include "complex_type.h"
#include "freqs.h"
#include "ring_buffer.h"

#define SZ_WINDOW 1250  // Samples

void fourier_initialize();
void fourier_update(float *amplitudes, uint16_t new_sample);

void fourier_print_ring_buffer();

#endif