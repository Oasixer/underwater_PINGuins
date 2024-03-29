#include "fourier.h"
#include <Arduino.h>

// Ring Buffer to hold the time domain
uint16_t ring_buffer[MAX_SZ_WINDOW] = {0};
uint16_t index_curr_in_ring_buffer = 0; 

uint16_t fourier_window_size = MAX_SZ_WINDOW;

// list to represent the factor to shift the fourier domain every update
complex_t shift_factor[N_FREQUENCIES];

complex_t shift_factor_for_N500[N_FREQUENCIES] = {
{0.97452687278657712521834355357, 0.224270760949381170457073153557},  // freq = 18000Hz, N = 500
{0.863923417192835274569517878263, 0.503623201635760797678642575192},  // freq = 42000Hz, N = 500
{0.951056516295153531181938433292, 0.309016994374947395751718204338},  // freq = 25000Hz, N = 500
{0.809016994374947451262869435595, 0.587785252292473137103456792829},  // freq = 50000Hz, N = 500
{0.899405251566371077842632075772, 0.437115766650932879855417922954},  // freq = 36000Hz, N = 500
{0.990461425696651187600139110145, 0.137790290684638078166912578126},  // freq = 11000Hz, N = 500

};


/* Because of rounding error accumulation, we have to have multiple 
frequency domains. We always update both, but every SZ_WINDOW samples, we reset one.
That way, one is always accurate and up to date. */
bool use_freq_domain_1_as_active_next_time = true;
complex_t freq_domain_1[N_FREQUENCIES] = {{0}};  // list to represent the fourier domain
complex_t freq_domain_2[N_FREQUENCIES] = {{0}};  // list to represent the fourier domain
complex_t *active_freq_domain = freq_domain_1;
complex_t *inactive_freq_domain = freq_domain_2;

void fourier_initialize(uint16_t N){
    if (N > MAX_SZ_WINDOW){
        Serial.printf("ERROR: Received window size (%i) is larger than the max. ignoring it.\n", N);
        return;
    } else {
        Serial.printf("Setting window size to %i\n", N);
    }
    fourier_window_size = N;

    // initialize the ring buffer to zeros
    for (uint16_t i = 0; i < MAX_SZ_WINDOW; i++){
        ring_buffer[i] = 0;
    }
    index_curr_in_ring_buffer = 0;

    // initialize the shift factor
    if (fourier_window_size == 500){
        for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
            shift_factor[i].real = shift_factor_for_N500[i].real;
            shift_factor[i].imaginary = shift_factor_for_N500[i].imaginary;
        }
    } else {
        Serial.printf("WARNING: The shift factors for this window size (%i) is not hardcoded. Calculating based on frequencies\n", fourier_window_size);
        uint16_t F_NATURAL = F_SAMPLE / fourier_window_size;
        for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
            uint16_t k = FREQUENCIES[i] / F_NATURAL;
            float theta = 2. * M_PI * k / fourier_window_size;
            shift_factor[i].real = cosf(theta);
            shift_factor[i].imaginary = sinf(theta);
        }
    }
    for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
        shift_factor[i].real = shift_factor_for_N500[i].real;
        shift_factor[i].imaginary = shift_factor_for_N500[i].imaginary;
    }

    // initialize the frequency domains to zeros
    for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
        freq_domain_1[i].real = 0;
        freq_domain_1[i].imaginary = 0;
        freq_domain_2[i].real = 0;
        freq_domain_2[i].imaginary = 0;
    }
}

void fourier_update(float *magnitudes, uint16_t new_sample){
    uint16_t old_sample = ring_buffer[index_curr_in_ring_buffer];
    ring_buffer[index_curr_in_ring_buffer] = new_sample; // overwrite old_sample

    if (++index_curr_in_ring_buffer == fourier_window_size){ // we reached end of ring buffer
        index_curr_in_ring_buffer = 0;  // reset to 0

        if (use_freq_domain_1_as_active_next_time){
            use_freq_domain_1_as_active_next_time = false;  // switch for next time
            active_freq_domain = freq_domain_1;
            inactive_freq_domain = freq_domain_2;
        } else {
            use_freq_domain_1_as_active_next_time = true;  // switch for next time
            active_freq_domain = freq_domain_2;
            inactive_freq_domain = freq_domain_1;
        }
        // reset inactive freq domain to zeros
        for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
            inactive_freq_domain[i] = {0};
        }
    }
    
    float difference = (float)(new_sample - old_sample);
    for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
        // add difference
        active_freq_domain[i].real += difference;
        inactive_freq_domain[i].real += new_sample;  // assume that old_sample is 0. so difference = new_sample
        
        // multiply by shift factor
        active_freq_domain[i] = {
            (active_freq_domain[i].real * shift_factor[i].real) - (active_freq_domain[i].imaginary * shift_factor[i].imaginary),
            (active_freq_domain[i].real * shift_factor[i].imaginary) + (active_freq_domain[i].imaginary * shift_factor[i].real),
        };
        inactive_freq_domain[i] = {
            (inactive_freq_domain[i].real * shift_factor[i].real) - (inactive_freq_domain[i].imaginary * shift_factor[i].imaginary),
            (inactive_freq_domain[i].real * shift_factor[i].imaginary) + (inactive_freq_domain[i].imaginary * shift_factor[i].real),
        };

        // calculate magnitude using active freq domain
        magnitudes[i] = sqrtf(
            active_freq_domain[i].real * active_freq_domain[i].real + 
            active_freq_domain[i].imaginary * active_freq_domain[i].imaginary);
    }
}

void fourier_print_ring_buffer(){
    for (uint16_t i = 0; i < MAX_SZ_WINDOW; i++){
        Serial.printf("%i, ", ring_buffer[i]);
    }
    Serial.println("");
}

void test_fourier_speed_main(){  
    unsigned long start_micros = micros();
    unsigned long fourier_counter = 0;
    float magnitudes[N_FREQUENCIES];
    fourier_initialize(500);
    while (micros() - start_micros < 10000000){
        fourier_update(magnitudes, 69);
        fourier_counter++;
    }
    Serial.printf("In 10 seconds, i managed to fourier %i times. My safety factor is %f\n", fourier_counter, ((float)fourier_counter)/5000000);
}
