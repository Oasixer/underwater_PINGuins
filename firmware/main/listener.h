#ifndef FYDP_LISTENER
#define FYDP_LISTENER

#include "constants.h"
#include "configurations.h"

typedef struct {
    bool finished;
    uint64_t ts_peak;
    uint8_t idx_identified_freq;
    float max_magnitude;
} listener_output_t;

class Listener{
    public:
        Listener(config_t *config);

        void begin(uint64_t ts_begin);
        void setup_adc();
        void end_adc_timer();
        void start_adc_timer();

        listener_output_t hb();//uint8_t listen_only_to=255);

    private:
        float* frequency_magnitudes;
        config_t *config;
        uint64_t ts_start_listening = 0;
        uint64_t ts_peak_finding_timeout = -1;
        uint64_t ts_peak = 0;
        uint8_t idx_identified_freq = 0;
        float curr_max_magnitude = 0;
        bool detected = false;
        float sum_of_freq_magnitdues_during_peak_finding[N_FREQUENCIES] = {0};
        IntervalTimer adc_timer; // for ADC read ISR @ intervals
};

#endif
