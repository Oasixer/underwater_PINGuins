#ifndef FYDP_ROV_MAIN_H
#define FYDP_ROV_MAIN_H

#include "tcp_client.h"
#include "listener.h"
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

        uint16_t n_round_robins_done = 0;

        uint64_t ts_stop_continuous_transmission = 0;
        uint64_t ts_response_timeout = 0;

        uint8_t idx_freq_detected = 0;
        uint8_t curr_expected_freq_idx = 0;

        float curr_max_magnitude = 0;

        float *frequency_magnitudes;
        // float **frequency_magnitudes[N_FREQUENCIES];
        // float *frequency_magnitudes;

        // configurations
        // extern config_t config;
        config_t* config;

        Listener* listener;
        // float measured_distances[MAX_N_TRIPS] = {0};

        uint16_t n_talks_done = 0;
        uint16_t n_talks_command = 0;
        
        uint16_t* last_reading;
        TcpClient* client;

        uint64_t trip_times[MAX_N_TRIPS][3] = {{0}};
        void receive_mode_hb(listener_output_t&);
        void send_mode_hb();
    public:
        RovMain(config_t* config, Listener* listener);
        void setup(TcpClient* client);
        void loop();
        void detect_frequencies();
        void reset_send_receive();
        void rov_peak_finding();
};
#endif
