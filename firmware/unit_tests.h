#ifndef FYDP_UNIT_TESTS_H
#define FYDP_UNIT_TESTS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "ring_buffer.h"
#include "fourier.h"
#include "complex_type.h"
#include "multilateration.h"
#include "positions.h"

bool test_all_algorithms();
bool test_ring_buffer_read_and_replace();
bool test_fourier();
bool test_complex_type();
bool test_multilateration();
bool test_calculate_coordinates();

#define MAX_FOURIER_ROUNDING_ERROR 200

#endif
