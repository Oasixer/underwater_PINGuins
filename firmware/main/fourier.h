#ifndef FYDP_FOURIER_H
#define FYDP_FOURIER_H

#include <stdint.h>
#include <math.h>
#include <stdio.h>

#include "complex_type.h"
#include "constants.h"

void fourier_initialize(uint16_t N);
void fourier_update(float *magnitudes, uint16_t new_sample);

void fourier_print_ring_buffer();
void test_fourier_speed_main();

#endif