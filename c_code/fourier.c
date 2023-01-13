#include "fourier.h"

ring_buffer_t ring_buffer = {0};  // Ring Buffer to hold the real domain

complex_t fourier_domain[N_FREQUENCIES] = {{0}};  // list to represent the fourier domain
complex_t shift_factor[N_FREQUENCIES];  // list to represent the factor to shift the fourier domain every update

void fourier_initialize(){
    uint16_t F_NATURAL = F_SAMPLE / SZ_WINDOW;
    for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
        uint16_t k = FREQUENCIES[i] / F_NATURAL;
        float theta = 2. * M_PI * k / SZ_WINDOW;
        shift_factor[i].real = cosf(theta);
        shift_factor[i].imaginary = sinf(theta);
    }
}

void fourier_update(float *amplitudes, uint16_t new_sample){
    uint16_t old_sample = ring_buffer_read_and_replace(&ring_buffer, new_sample);
    float difference = (float)(new_sample - old_sample);
    for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
        fourier_domain[i] = complex_add_real(fourier_domain[i], difference);
        fourier_domain[i] = complex_multiply_complex(fourier_domain[i], shift_factor[i]);
        amplitudes[i] = complex_magnitude(fourier_domain[i]);
    }
}

void fourier_print_ring_buffer(){
    for (uint16_t i = 0; i < RING_BUFFER_SIZE; i++){
        printf("%i,\n", ring_buffer.data[i]);
    }
    printf("\n");
}
