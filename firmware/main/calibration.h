#ifndef FYDP_CALIBRATION_H
#define FYDP_CALIBRATION_H

#include "utils.h" // for coord_3d_t
#include "listener.h"
#include "talker.h"
#include "tcp_client.h"
#include "configurations.h"
#include "positions.h"
#include "constants.h"

typedef struct{
    uint64_t cycle_time_first_response;
    uint64_t cycle_time_second_response;
} cycle_node_result_t;

typedef struct{
    cycle_node_result_t node_results[3];
    // N_ALL_NODES is 4 fyi
    distances_t dists[N_ALL_NODES]; // each distance_t is 4 floats, one for each other node/rov
} cycle_result_t;

typedef struct{
    uint16_t retries[3]; // retries by node
    uint16_t cycle_count;
    cycle_result_t cycle_results[MAX_CALIBRATION_CYCLES];
    distances_t averaged_distances[N_ALL_NODES]; // each distance_t is 4 floats, one for each other node/rov
    coord_3d_t node_coords_3d[N_ALL_NODES];
} calibration_data_t;

class Calibration {
    public:
        bool is_calibrated;
        coord_3d_t coords[N_ALL_NODES];
        bool has_depths;
        float depths[N_ALL_NODES];
        Calibration();
        Calibration(TcpClient* _client,
                    Listener* _listener,
                    Talker* _talker,
                    config_t* _config);
                    

        bool calibration_in_progress_tick(); // return true if done
        void manual_calibration_coords(const String token, const float depth);
        calibration_data_t* begin(uint16_t _n_cycles_cmd);
        float trip_time_to_dist(uint64_t trip_time);
        void Calibration::set_manual_depths(const String depths_str);
    private:
        TcpClient* client;
        Listener* listener;
        Talker* talker;
        config_t* config;
        uint64_t ts_cycle_node_start;
        uint64_t ts_listen_start;
        uint64_t ts_listen_timeout;
        uint8_t calibrating_node_135; // 1, then 3, then 5
        uint8_t start_yell_listen_x2_0123; // 0 then 1 then 2 then 3
                                           // for states in name respectively
        // cycle_result_t cycle_results[MAX_CALIBRATION_CYCLES];
        // uint16_t cycle_count;
        uint16_t n_cycles_cmd;
        calibration_data_t cal_data;

        bool incorrect_frequency(uint8_t, uint8_t);
        bool save_result_and_return_should_increment_node(listener_output_t* result, uint8_t node_id_to_listen_for);
        bool increment_node_and_return_should_increment_cycle();
        bool increment_cycle_return_is_calibration_done();
        bool listen_return_is_calibration_done();

        coord_3d_t get_coord_from_string(String str);
        uint64_t yell_to_listen_offset();
        uint64_t listen_start_to_timeout_offset();

        // void trip_times_to_calibration_dists(distances_t dists[N_ALL_NODES], cycle_node_result_t measured_times[3]);
        void resolve_cycle_dists();
        // void convert_trip_times_to_dists();
        void average_dists_and_get_coords();
        void update_rov_depth_50ms();
};

#endif