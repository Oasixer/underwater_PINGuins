#include "listener.h"
#include <Arduino.h>

Listener::Listener(config_t *config){
    this->config = config;
}

void Listener::begin(uint64_t ts_begin){
    this->ts_start_listening = ts_begin;
    ts_peak_finding_timeout = -1;
    ts_peak = 0;
    idx_identified_freq = 0;
    curr_max_magnitude = 0.0;
    detected = false;
    for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
        sum_of_freq_magnitdues_during_peak_finding[i] = 0.0;
    }

}

listener_output_t Listener::hb(){
    if (micros() >= ts_start_listening){  // if not in inactive period
        for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
            if (frequency_magnitudes[i] > curr_max_magnitude){
                curr_max_magnitude = frequency_magnitudes[i];
                idx_identified_freq = i;
               
                // first detected a frequency
                if (!detected && curr_max_magnitude > config->dft_threshold){
                    detected = true;
                    ts_peak = micros();  // initialize ts_peak to when first detected (rising edge)
                    // initialize peak finding timeout
                    ts_peak_finding_timeout = micros() + config->duration_to_find_peak; 
                }

                if (!config->use_rising_edge){ // use peak time
                    ts_peak = micros(); // update ts peak to the time of largest value
                }
            }
        }

        if (detected) {
            if (config->integrate_freq_domain) {
                for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
                    sum_of_freq_magnitdues_during_peak_finding[i] += frequency_magnitudes[i];
                }
            }

            if (micros() >= ts_peak_finding_timeout) {
                float largest_sum = 0.0;
                if (config->integrate_freq_domain){
                    for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
                        if (sum_of_freq_magnitdues_during_peak_finding[i] > largest_sum){
                            largest_sum = sum_of_freq_magnitdues_during_peak_finding[i];
                            idx_identified_freq = i;
                        }
                    }
                }
                return {true, ts_peak, idx_identified_freq};
            }
        }
    }
    return {false, 0, 0};
}