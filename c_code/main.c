#include <stdio.h>
#include "complex_type.h"
#include "ring_buffer.h"
#include "fourier.h"
#include "frequencies.h"

int main() {
    complex_t z = {1, 4};
    complex_add_real(&z, 2.0);
    printf("%.2f + %.2fI", z.real, z.imaginary);

    ring_buffer_t ring_buffer = RING_BUFFER_DEFAULT;
    ring_buffer_read(&ring_buffer);
//
//    for (size_t i = 0; i < SZ_WINDOW; ++i){
//        float t = i / F_SAMPLE;
//        uint8_t input = 2048.0*sinf(2.0*M_PI * F_LOW * t) + 2048.0;
//        update_fourier(input);
//        printf("%i: input = %i, ", i, input);
//        for (size_t j = 0; j < N_FREQUENCIES; ++j){
//            printf("%f.2 + %f.2 , ", fourier_domain[j].real, fourier_domain[j].imaginary);
//        }
//        printf("\n");
//    }
    return 0;
}
