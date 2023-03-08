#ifndef FYDP_ROV_MAIN_H
#define FYDP_ROV_MAIN_H

#include "tcp_client.h"
#include "listener.h"
#include "constants.h"
#include "configurations.h"
#include "calibration.h"
#include "utils.h"

class RovMain {
    private:
        // for receiving
        // IntervalTimer adc_timer; // for ADC read ISR @ intervals

        // state variables
        bool is_currently_receiving = false;  // if true receiving data. else sending
        bool is_transmit_continuously = false;  // set by user commands

        uint64_t ts_start_talking = 0;
        uint64_t ts_stop_continuous_transmission = 0;
        uint64_t ts_response_timeout = 0;

        uint16_t n_round_robins_done = 0;
        uint16_t n_round_robins_command = 0;
        uint16_t n_talks_done = 0;
        uint16_t n_talks_command = 0;
        uint16_t frequency_to_send = 18000;

        uint8_t curr_freq_idx = 0;

        coord_3d_t node_coords_3d[N_ALL_NODES] = {{0}};

        float *frequency_magnitudes;
        float og_depth = 0.0;
        float curr_depth = 0.0;

        config_t* config;

        Listener* listener;
        Talker* talker;
        Calibration calibration;
        calibration_data_t* calibration_data;
        
        uint16_t* last_reading;
        TcpClient* client;

        uint64_t trip_times_round_robin[3] = {{0}};
        uint64_t trip_times_single_freq[MAX_N_TRIPS] = {{0}};
        void round_robin_receive_mode_hb(listener_output_t&);
        void receive_mode_hb_single_freq(listener_output_t &listener_data);
        void send_mode_hb();
        float trip_time_to_dist(uint64_t trip_time);
        void check_if_done_round_robins();
        bool is_calibrating = false;
    public:
        RovMain(config_t* config, Listener* listener, TcpClient* client, Talker* talker);
        void setup();
        void shutdown();
        bool loop();
        void detect_frequencies();
        void reset_send_receive();
        void rov_peak_finding();
};
#endif
