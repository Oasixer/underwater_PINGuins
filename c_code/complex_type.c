#include "complex_type.h"

complex_t complex_multiply_complex(complex_t c1, complex_t c2){
    complex_t product = {
            (c1.real * c2.real) - (c1.imaginary * c2.imaginary),
            (c1.real * c2.imaginary) + (c1.imaginary * c2.real),
    };
    return product;
}

complex_t complex_add_real(complex_t c1, float real){
    complex_t sum = {
            c1.real + real,
            c1.imaginary,
    };
    return sum;
}

float complex_magnitude(complex_t c){
    return sqrtf(powf(c.real, 2) + powf(c.imaginary, 2));
}