#ifndef FYDP_STATIONARY_MAIN_H
#define FYDP_STATIONARY_MAIN_H

#include "tcp_client.h"
#include "constants.h"
#include "configurations.h"
#include "listener.h"

// void stationary_main_setup(TcpClient& client);
// void stationary_main_loop(TcpClient& client);



class StationaryMain {
    private:
// state variables
        bool is_currently_receiving = true;  // if true receiving data. else sending
        bool is_peak_finding = false;  // to indicate peak finding state
        bool listen_for_call_and_respond = true;  // set by user commands

        uint64_t ts_start_listening = 0;
        uint64_t ts_start_talking = 0;
        uint64_t ts_peak = 0;
        uint64_t ts_peak_finding_timeout = 0;

        uint8_t idx_freq_detected = 0;

        float curr_max_magnitude = 0;

        IntervalTimer adc_timer; // for ADC read ISR @ intervals
        float* frequency_magnitudes;

        // variables used for checking health
        // uint64_t *fourier_counter;
        uint16_t *last_reading;
        uint64_t t_last_printed = 0;

        TcpClient* client;
        // configurations
        config_t* config;

        Listener* listener;
        // void detect_frequencies();
        void peak_finding();
        void receive_mode_hb();
        void send_data();
        void send_mode_hb();
        void reply_yell();
    public:
        StationaryMain(config_t* config, Listener* listener, TcpClient* client);
        void setup();

        void shutdown();
        bool loop();
};

// #endif // FYDP_STATIONARY_MAIN_H

#endif