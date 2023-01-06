#ifndef FYDP_FOURIER_H
#define FYDP_FOURIER_H

#include <stdint.h>
#include <math.h>

#include "complex_type.h"
#include "freqs.h"
#include "ring_buffer.h"

#define SZ_WINDOW 1250;  // Samples

void initialize_fourier();
void update_fourier(complex_t *fourier_domain, uint8_t new_sample);

#endif