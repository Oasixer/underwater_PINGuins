#ifndef FYDP_ROV_MAIN_H
#define FYDP_ROV_MAIN_H

#include "tcp_client.h"
#include "constants.h"
#include "configurations.h"

class RovMain {
    private:
        // for receiving
        IntervalTimer adc_timer; // for ADC read ISR @ intervals

        // state variables
        bool is_currently_receiving = false;  // if true receiving data. else sending
        bool is_peak_finding = false;  // to indicate peak finding state
        bool is_transmit_continuously = false;  // set by user commands

        uint64_t ts_start_listening = 0;
        uint64_t ts_start_talking = 0;
        uint64_t ts_peak = 0;
        uint64_t ts_peak_finding_timeout = 0;

        uint64_t ts_stop_continuous_transmission = 0;
        uint64_t ts_response_timeout = 0;

        uint8_t idx_freq_detected = 0;

        float curr_max_magnitude = 0;

        float *frequency_magnitudes[N_FREQUENCIES];
        // float **frequency_magnitudes[N_FREQUENCIES];
        // float *frequency_magnitudes;

        // configurations
        // extern config_t config;
        config_t* config;

        float measured_distances[MAX_N_TRIPS] = {0};

        uint64_t trip_times[MAX_N_TRIPS] = {0};
        uint16_t n_talks_done = 0;
        uint16_t n_talks_command = 0;
        
        TcpClient* client;
    public:
        RovMain();
        void setup(TcpClient* client, config_t* config);
        void loop();
        void detect_frequencies();
        void rov_send_mode_hb();
        void reset_send_receive();
        void rov_peak_finding();
};
#endif
