#ifndef FYDP_CONFIGURATIONS
#define FYDP_CONFIGURATIONS

#include <stdint.h>

typedef struct{
    uint16_t fourier_window_size;
    uint32_t dft_threshold;
    uint16_t my_frequency;
    uint16_t micros_to_find_peak;
    uint16_t micros_send_duration;
} config_t;

config_t config = {500, 5000, 18000, 1000, 1000};

#endif
