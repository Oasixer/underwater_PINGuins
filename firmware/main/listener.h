// #ifndef
// class AdcHandler{

// }
#ifndef FYDP_LISTENER_H
#define FYDP_LISTENER_H

#include "constants.h"
#include "complex_type.h"
#include "tcp_client.h"
#include <ADC.h>

class Listener{
    private:
        float frequency_magnitudes[N_FREQUENCIES];
        uint64_t fourier_counter = 0;
        uint16_t last_reading = 0;
        ADC *adc; 

        TcpClient* client;

        uint16_t ring_buffer[MAX_SZ_WINDOW] = {0};
        uint16_t index_curr_in_ring_buffer = 0; 
        uint16_t fourier_window_size = MAX_SZ_WINDOW;

        // list to represent the factor to shift the fourier domain every update
        complex_t shift_factor[N_FREQUENCIES];

        complex_t temp_shift_factor[N_FREQUENCIES] = {
        {0.990461425696651187600139110145, 0.137790290684638078166912578126},  // freq = 11000Hz, N = 500
        {0.97452687278657712521834355357, 0.224270760949381170457073153557},  // freq = 18000Hz, N = 500
        {0.951056516295153531181938433292, 0.309016994374947395751718204338},  // freq = 25000Hz, N = 500
        {0.899405251566371077842632075772, 0.437115766650932879855417922954},  // freq = 36000Hz, N = 500
        {0.863923417192835274569517878263, 0.503623201635760797678642575192},  // freq = 42000Hz, N = 500
        {0.809016994374947451262869435595, 0.587785252292473137103456792829},  // freq = 50000Hz, N = 500

        };

        complex_t shift_factor_for_N500[N_FREQUENCIES] = {
            {0.992114701314477875904174197785, 0.125333233564304258322863461217},  // freq = 10000Hz, N = 500
            {0.97452687278657712521834355357, 0.224270760949381170457073153557},  // freq = 18000Hz, N = 500
            {0.947098304994744344398327484669, 0.320943609807209484507950492116},  // freq = 26000Hz, N = 500
            {0.910105970684995657649096756359, 0.414375580993284142738986020049},  // freq = 34000Hz, N = 500
            {0.863923417192835274569517878263, 0.503623201635760797678642575192},  // freq = 42000Hz, N = 500
            {0.823532597628427454061750268011, 0.567268949126756516498915061675},  // freq = 48000Hz, N = 500
        };
        complex_t shift_factor_for_N250[N_FREQUENCIES] = {
            {0.992114701314477875904174197785, 0.125333233564304258322863461217},  // freq = 10000Hz, N = 250
            {0.97452687278657712521834355357, 0.224270760949381170457073153557},  // freq = 18000Hz, N = 250
            {0.947098304994744344398327484669, 0.320943609807209484507950492116},  // freq = 26000Hz, N = 250
            {0.910105970684995657649096756359, 0.414375580993284142738986020049},  // freq = 34000Hz, N = 250
            {0.863923417192835274569517878263, 0.503623201635760797678642575192},  // freq = 42000Hz, N = 250
            {0.823532597628427454061750268011, 0.567268949126756516498915061675},  // freq = 48000Hz, N = 250
        };
        complex_t shift_factor_for_N1000[N_FREQUENCIES] = {
            {0.992114701314477875904174197785, 0.125333233564304258322863461217},  // freq = 10000Hz, N = 1000
            {0.97452687278657712521834355357, 0.224270760949381170457073153557},  // freq = 18000Hz, N = 1000
            {0.947098304994744344398327484669, 0.320943609807209484507950492116},  // freq = 26000Hz, N = 1000
            {0.910105970684995657649096756359, 0.414375580993284142738986020049},  // freq = 34000Hz, N = 1000
            {0.863923417192835274569517878263, 0.503623201635760797678642575192},  // freq = 42000Hz, N = 1000
            {0.823532597628427454061750268011, 0.567268949126756516498915061675},  // freq = 48000Hz, N = 1000
        };
        complex_t shift_factor_for_N1250[N_FREQUENCIES] = {
            {0.992114701314477875904174197785, 0.125333233564304258322863461217},  // freq = 10000Hz, N = 1250
            {0.97452687278657712521834355357, 0.224270760949381170457073153557},  // freq = 18000Hz, N = 1250
            {0.947098304994744344398327484669, 0.320943609807209484507950492116},  // freq = 26000Hz, N = 1250
            {0.910105970684995657649096756359, 0.414375580993284142738986020049},  // freq = 34000Hz, N = 1250
            {0.863923417192835274569517878263, 0.503623201635760797678642575192},  // freq = 42000Hz, N = 1250
            {0.823532597628427454061750268011, 0.567268949126756516498915061675},  // freq = 48000Hz, N = 1250
        };


        /* Because of rounding error accumulation, we have to have multiple 
        frequency domains. We always update both, but every SZ_WINDOW samples, we reset one.
        That way, one is always accurate and up to date. */
        bool use_freq_domain_1_as_active_next_time = true;
        complex_t freq_domain_1[N_FREQUENCIES] = {{0}};  // list to represent the fourier domain
        complex_t freq_domain_2[N_FREQUENCIES] = {{0}};  // list to represent the fourier domain
        complex_t *active_freq_domain = freq_domain_1;
        complex_t *inactive_freq_domain = freq_domain_2;
        
        void adc_isr();

    public:
        Listener(TcpClient* client);
        void fourier_update(uint16_t reading);
};
#endif