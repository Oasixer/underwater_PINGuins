#include "fourier.h"

complex_t shift_factor[N_FREQUENCIES];

ring_buffer_t ring_buffer = RING_BUFFER_DEFAULT;

void initialize_fourier(){
    uint16_t F_NATURAL = F_SAMPLE / SZ_WINDOW;
    uint16_t k[N_FREQUENCIES] = {
        F_LOW / F_NATURAL,
        F_HIGH / F_NATURAL,
        F_STATIONARY[0] / F_NATURAL,
        F_STATIONARY[1] / F_NATURAL,
        F_STATIONARY[2] / F_NATURAL,
        F_ALL / F_NATURAL,
    };
    for (size_t i = 0; i < N_FREQUENCIES; ++i){
        float theta = 2. * M_PI * k[i] / SZ_WINDOW;
        shift_factor[i].real = cosf(theta);
        shift_factor[i].imaginary = sinf(theta);
    }
}

void update_fourier(uint8_t new_sample){
    uint8_t old_sample = ring_buffer_read(&ring_buffer);
    uint8_t difference = new_sample - old_sample;
    for (size_t i = 0; i < N_FREQUENCIES; ++i){
        complex_add_real(&fourier_domain[i], difference);
        complex_multiply_complex(&fourier_domain[i], shift_factor[i]);
    }
}