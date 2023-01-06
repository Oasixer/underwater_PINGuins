#include "complex_type.h"

void complex_multiply_complex(complex_t *c1, complex_t c2){
    c1->real = (c1->real * c2.real) - (c1->imaginary * c2.imaginary);
    c1->imaginary = (c1->real * c2.imaginary) - (c1->imaginary * c2.real);
}

void complex_add_real(complex_t *c1, float real){
    c1->real += real;
}

float complex_magnitude(complex_t *c){
    return sqrtf(c->real*c->real + c->imaginary*c->imaginary);
}