#ifndef COMPLEX_H
#define COMPLEX_H

#include <float.h>
#include <math.h>

typedef struct{
    float real;
    float imaginary;
} complex_t;

void complex_multiply_complex(complex_t *c1, complex_t c2);
void complex_add_real(complex_t *c1, float real);

float complex_magnitude(complex_t *c);

#endif