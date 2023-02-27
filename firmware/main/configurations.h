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
    uint16_t micros_to_find_peak;
    uint16_t micros_send_duration;
    uint32_t response_timeout_duration;
    bool use_rising_edge;
} config_t;

config_t config = {500, 25000, 18000, 1000, 1000, 400000, true};

#endif
