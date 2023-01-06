#include <stdio.h>

#include "complex_type.h"
#include "ring_buffer.h"
#include "fourier.h"
#include "freqs.h"
#include "utils.h"

int main() {
    printf("hello world, %i\n", F_LOW);
    complex_t z = {1, 4};
    complex_add_real(&z, 2.0);
    printf("%.2f + %.2fI\n", z.real, z.imaginary);

    ring_buffer_t ring_buffer = RING_BUFFER_DEFAULT;
    uint8_t first_read = ring_buffer_read(&ring_buffer);
    printf("first read %i\n", first_read);

    coord_2d_t c1 = {0, 0};
    coord_2d_t c2 = {3, 4};

    printf("dist %.2f\n", dist_between_points_2d(c1, c2));
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
