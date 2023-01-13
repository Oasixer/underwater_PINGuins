#include "unit_tests.h"

bool test_ring_buffer_read_and_replace(){
    ring_buffer_t ring_buffer = {0};

    bool is_initialization_good = true;
    for (uint16_t i = 0; i < RING_BUFFER_SIZE; ++i){
        is_initialization_good &= ring_buffer.data[i] == 0;
    }
    is_initialization_good &= ring_buffer.i_curr == 0;
    if (is_initialization_good){
        printf("Ring buffer Initialization Test Passed\n");
    } else {
        printf("Ring buffer Initialization Test Failed\n");
    }

    bool is_read_and_replace_good = true;
    for (uint16_t i = 0; i < RING_BUFFER_SIZE; ++i){
        uint16_t curr = ring_buffer_read_and_replace(&ring_buffer, i % 256);
        is_read_and_replace_good &= curr == 0;
        is_read_and_replace_good &= ring_buffer.i_curr == (i+1) % RING_BUFFER_SIZE;
    }
    for (uint16_t i = 0; i < RING_BUFFER_SIZE; ++i){
        uint16_t curr = ring_buffer.data[i];
        is_read_and_replace_good &= curr == i % 256;
    }
    if (is_read_and_replace_good){
        printf("Ring buffer Read and Replace Test Passed\n");
    } else {
        printf("Ring buffer Read and Replace Test Failed\n");
    }

    bool all_tests_pass = is_read_and_replace_good && is_initialization_good;
    if (all_tests_pass){
        printf("Ring buffer all tests Passed\n\n");
    } else {
        printf("Ring buffer tests Failed\n\n");
    }

    return all_tests_pass;
}

bool test_complex_type(){
    complex_t c1 = {3, 4};
    bool is_magnitude_working = complex_magnitude(c1) == 5.0;
    if (is_magnitude_working){
        printf("Complex Magnitude Test Passed\n");
    } else {
        printf("Complex Magnitude Test Failed\n");
    }

    complex_t sum = complex_add_real(c1, 5);
    bool is_addition_working = sum.real == 8 && sum.imaginary == 4;
    if (is_addition_working){
        printf("Complex Addition Test Passed\n");
    } else {
        printf("Complex Addition Test Failed\n");
    }

    complex_t c2 = {6, -9};
    complex_t product = complex_multiply_complex(c1, c2);
    bool is_multiplication_woring = (fabsf(product.real - 54) < 1e-5) && (fabsf(product.imaginary - -3) < 1e-5);
    if (is_multiplication_woring){
        printf("Complex Multiplication Test Passed\n");
    } else {
        printf("Complex Multiplication Test Failed\n");
    }

    bool all_tests_pass = is_magnitude_working && is_addition_working && is_multiplication_woring;
    if (all_tests_pass){
        printf("Complex all tests Passed\n\n");
    } else {
        printf("Complex tests Failed\n\n");
    }

    return all_tests_pass;
}

bool test_fourier(){
    fourier_initialize();

    float amplitudes[N_FREQUENCIES] = {0};

    // Test a low input
    bool is_no_signal_working = true;
    for (uint16_t i = 0; i < SZ_WINDOW; ++i){
        fourier_update(amplitudes, 2048.0);
    }
    for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
        is_no_signal_working &= amplitudes[i] < 100; // should be low
    }
    if (is_no_signal_working){
        printf("Fourier No Signal Test Passed\n");
    } else {
        printf("Fourier No Signal Test Failed\n");
    }

    // Test frequencies one by one as high
    bool is_individual_working = true;
    for (uint8_t i_freq = 0; i_freq < N_FREQUENCIES; ++ i_freq){
        for (uint16_t i = 0; i < SZ_WINDOW; ++i){
            float t = (float)i / F_SAMPLE;
            uint16_t x = (uint16_t)roundf(2048.0 * sinf(2 * M_PI * FREQUENCIES[i_freq] * t) + 2048.0);
            fourier_update(amplitudes, x);
        }
        for (int j = 0; j < 6; ++j){
            if (i_freq != j){  // should be low
                is_individual_working &= amplitudes[j] < 100;
            } else {  // should be high
                is_individual_working &= (amplitudes[j] - 1280000) < 100;
            }
        }
    }
    if (is_individual_working){
        printf("Fourier Individual Test Passed\n");
    } else {
        printf("Fourier Individual Test Failed\n");
    }

    // test all frequencies high
    bool is_multiple_frequencies_high_working = true;
    for (uint16_t i = 0; i < SZ_WINDOW; ++i){
        float t = (float)i / F_SAMPLE;
        float input = 0;
        for (uint8_t i_freq = 0; i_freq < N_FREQUENCIES; ++i_freq){
            input += (2048.0 * sinf(2 * M_PI * FREQUENCIES[i_freq] * t) + 2048.0) / N_FREQUENCIES;
        }
        uint16_t x = (uint16_t)(roundf(input));
        fourier_update(amplitudes, x);
    }
    for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
        is_multiple_frequencies_high_working &= fabs(amplitudes[i] - 213333.333333) < 100; // should be high
    }
    if (is_multiple_frequencies_high_working){
        printf("Fourier Multiple High Test Passed\n");
    } else {
        printf("Fourier Multiple High Test Failed\n");
    }

    bool all_tests_pass = is_no_signal_working && is_individual_working && is_multiple_frequencies_high_working;
    if (all_tests_pass){
        printf("Fourier all tests Passed\n\n");
    } else {
        printf("Fourier tests Failed\n\n");
    }

    return all_tests_pass;
}
