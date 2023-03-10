#ifndef FYDP_CONFIGURATIONS
#define FYDP_CONFIGURATIONS

#include "constants.h"
#include <stdint.h>
#include <stdbool.h>
#include <IntervalTimer.h>

typedef struct{
    uint16_t fourier_window_size;
    uint32_t dft_threshold;
    uint16_t duration_to_find_peak;
    uint16_t micros_send_duration;
    uint32_t response_timeout_duration;
    uint32_t period;
    float speed_of_sound;
    float marco_polo_time_delay;
    bool use_rising_edge;
    bool integrate_freq_domain;
    bool use_pressure_sensor;
} config_t;


#endif
