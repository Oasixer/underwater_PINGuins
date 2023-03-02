#ifndef FYDP_LISTENER
#define FYDP_LISTENER

#include "constants.h"
#include "configurations.h"

class Listener{
    public:
        Listener(config_t *config);

        void begin(uint64_t ts_begin);

        listener_output_t hb();

    private:
        float frequency_magnitudes[N_FREQUENCIES];
        config_t *config;
        uint64_t ts_peak_finding_timeout = -1;
        uint64_t ts_start_listening = 0;
        uint64_t ts_peak = 0;
        uint8_t idx_identified_freq = 0;
        float sum_of_freq_magnitdues_during_peak_finding[N_FREQUENCIES] = {0};
        float curr_max_magnitude = 0;
        bool is_peak_finding = false;
};

typedef struct {
    bool finished;
    uint64_t ts_peak;
    uint8_t idx_identified_freq;
} listener_output_t;

#endif
