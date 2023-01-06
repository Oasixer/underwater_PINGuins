#ifndef FOURIER_H
#define FOURIER_H

#include <stdint.h>
#include <math.h>

#include "complex_type.h"
#include "frequencies.h"
#include "ring_buffer.h"

const uint16_t SZ_WINDOW = 1250;  // Samples

complex_t fourier_domain[N_FREQUENCIES] = {{0}};

void initialize_fourier();
void update_fourier(uint8_t new_sample);

#endif