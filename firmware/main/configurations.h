#ifndef FYDP_CONFIGURATIONS
#define FYDP_CONFIGURATIONS

#include "constants.h"
#include <stdint.h>
#include <stdbool.h>
#include <IntervalTimer.h>

typedef struct{
    uint16_t fourier_window_size;
    uint32_t dft_threshold;
    uint16_t my_frequency;
    uint16_t duration_to_find_peak;
    uint16_t micros_send_duration;
    uint32_t response_timeout_duration;
    uint32_t period;
    float speed_of_sound;
    float marco_polo_time_delay;
    bool use_rising_edge;
    bool integrate_freq_domain;
} config_t;

config_t config = {
    500, // fourier_window_size
    25000,  // dft_threshold
    18000,  // my_frequency
    1000,  // duration_to_find_peak
    1000,  // micros_send_duration
    400000,  // response_timeout_duration
    200000,  // period
    1351.89889440064462178270, // speed_of_sound
    200056.07745910956873558462, // marco_polo_time_delay
    true, // use_rising_edge
    false, // integrate_freq_domain
};

#endif
