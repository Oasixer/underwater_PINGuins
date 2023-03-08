#include "calibration.h"
#include "positions.h"
#include "dac_driver.h"
#include "relay.h"
#include "utils.h"
#include "MS5839.h"


Calibration::Calibration(){   
}

Calibration::Calibration(TcpClient* _client,
                         Listener* _listener,
                         Talker* _talker,
                         config_t* _config){
    is_calibrated = false;
    has_depths = false;
    calibrating_node_135 = 1;
    start_yell_listen_x2_0123 = 0;
    cal_data.cycle_count = 0;
    n_cycles_cmd = MAX_CALIBRATION_CYCLES; // will be set to a lower number at begin()
    for (uint8_t i=0; i<N_ALL_NODES; i++){
        depths[i] = 0;
    }

    client = _client;
    listener = _listener;
    talker = _talker;
    config = _config;
}

calibration_data_t* Calibration::begin(uint16_t _n_cycles_cmd){
    n_cycles_cmd = _n_cycles_cmd;
    is_calibrated = false;
    if (!has_depths){
        client->print("Beginning calibration. No node depths provided, assuming they are at the ROV's depth.\n");
    }

    // calibrating node 1, will be updated to 3 then 5 before advancing to next cycle
    calibrating_node_135 = 1; 
    // state management. 0= start, 1= send, 2= listen first reply, 3= listen second reply
    start_yell_listen_x2_0123 = 0;


    for (uint16_t i=0; i<MAX_CALIBRATION_CYCLES; i++){
        for (uint8_t j=0; j<3; j++){
            // cal_data.cycle_results[i].n_retries = 0;
            cal_data.cycle_results[i].node_results[j].cycle_time_first_response = 0;
            cal_data.cycle_results[i].node_results[j].cycle_time_second_response = 0;
        }
        for (uint8_t j=0; j<N_ALL_NODES; j++){
            for (uint8_t k=0; k<N_ALL_NODES; k++){
                cal_data.cycle_results[i].dists[j].dist[k] = 0;
            }
        }
    }

    cal_data.cycle_count = 0;

    return &cal_data;
}

uint64_t Calibration::yell_to_listen_offset(){
    return config->micros_send_duration + config->period - MICROS_TO_LISTEN_BEFORE_END_OF_PERIOD;
}


coord_3d_t get_coord_from_string(String& str){
    int idx_delimiter_1 = str.indexOf(',');
    int idx_delimiter_2 = str.indexOf(',', idx_delimiter_1 + 1);
    return coord_3d_t{
        float(atof(str.substring(0, idx_delimiter_1).c_str())),
        float(atof(str.substring(idx_delimiter_1 + 1, idx_delimiter_2).c_str())),
        float(atof(str.substring(idx_delimiter_2 + 1, str.length()).c_str())),
    };
}

bool Calibration::calibration_in_progress_tick(){
    // start_yell_listen_x2_0123 is a state machine.
    //  ->  0= start, 1= send, 2= listen first reply, 3= listen second reply
    if (start_yell_listen_x2_0123 == 0){ // starting node
        cycle_result_t* cycle_result = &cal_data.cycle_results[cal_data.cycle_count];
        cycle_node_result_t* node_result = &cycle_result->node_results[freq_idx_to_node_n(calibrating_node_135)];
        node_result->cycle_time_first_response = 0;
        node_result->cycle_time_second_response = 0;
        ts_cycle_node_start = talker->begin_set_relay_return_start_time(calibrating_node_135);
        ts_listen_start = ts_cycle_node_start - yell_to_listen_offset();
        return false; // only return true when done calibration
    }
    else if (start_yell_listen_x2_0123 == 1){ // yelling
        bool yell_completed = talker->yell();
        if (yell_completed){ 
            start_yell_listen_x2_0123 = 2;
            switch_relay_to_receive_6ms();
            update_rov_depth_50ms(); // blocking stuff
            listener->begin(ts_listen_start);
            listener->start_adc_timer();
        }
        return false; // only return true when done calibration
    }
    else if (start_yell_listen_x2_0123 == 2 || start_yell_listen_x2_0123 == 3){ // listening
        // perform listening tick. if calibration done, returns true which we propagate up
        if (listen_return_is_calibration_done()){
            is_calibrated = true;
            average_dists_and_get_coords();
            return true; // return true = done calibration
        }
    }
    else{
        client->print("ERROR: Bad code brought start_send_listen_x2_0123_0123 out of range to "+String(start_yell_listen_x2_0123)+"\n");
    }
    return false; // only return true when done calibration
}

bool Calibration::listen_return_is_calibration_done(){ 
    uint8_t node_id_to_listen_for = (calibrating_node_135 + 1) % 6;
    listener_output_t result = listener->hb();
    if (result.finished){
        // incorrect_frequency will client-> print a message if error
        if (save_result_and_return_should_increment_node(&result, node_id_to_listen_for)){
            if (increment_node_and_return_should_increment_cycle()){
                resolve_cycle_dists(); // convert times to dists
                if (increment_cycle_return_is_calibration_done()){
                    return true; // done calibration, return true
                }
            }
        }
    }
    return false; // only return true when done calibration
}

bool Calibration::increment_cycle_return_is_calibration_done(){
    if (cal_data.cycle_count + 1 >= n_cycles_cmd){ // add 1 bc 0-indexed and we haven't incremented yet
        return true;
    }
    cal_data.cycle_count++;
    return false;
}

bool Calibration::increment_node_and_return_should_increment_cycle(){ // return true if finished all cycles
    start_yell_listen_x2_0123 = 0;
    if (calibrating_node_135 == 5){
        // cal_data.cycle_count += 1;
        calibrating_node_135 = 1;
        return true;
    }
    calibrating_node_135 += 2;
    return false;
}

bool Calibration::save_result_and_return_should_increment_node(listener_output_t* result, uint8_t node_id_to_listen_for){
    bool retry_due_to_fail = incorrect_frequency(result->idx_identified_freq, node_id_to_listen_for);
    if (retry_due_to_fail){
        start_yell_listen_x2_0123 = 0;
        cal_data.retries[freq_idx_to_node_n(calibrating_node_135)] += 1;
        return false;
    }

    cycle_result_t* cycle_result = &cal_data.cycle_results[cal_data.cycle_count];
    cycle_node_result_t* node_result = &cycle_result->node_results[freq_idx_to_node_n(calibrating_node_135)];

    uint64_t cycle_time = result->ts_peak - ts_cycle_node_start;
    if (start_yell_listen_x2_0123 == 2){
        node_result->cycle_time_first_response = cycle_time;
        start_yell_listen_x2_0123++;
        return false;
    }
    else if (start_yell_listen_x2_0123 == 3){
        node_result->cycle_time_second_response = cycle_time;
        start_yell_listen_x2_0123 = 0; // set in increment_node_... but this is here for explicitness
        return true;
    }
    client->print("ERROR: I wrote bad code... how can start_send_listen be"+String(start_yell_listen_x2_0123)+"?\n");
    return false; // if there is bad code, keep retrying till a human notices i guess
}

bool Calibration::incorrect_frequency(uint8_t idx_identified_freq, uint8_t node_id_to_listen_for){
    if (idx_identified_freq != node_id_to_listen_for){
        char msg[80];
        sprintf(msg, "ERROR: expected to hear id %u, but heard id %u\n", node_id_to_listen_for, idx_identified_freq);
        client->print(String(msg));
        return true;
    }
    return false;
}

float Calibration::trip_time_to_dist(uint64_t trip_time){
    return ((float)trip_time - config->marco_polo_time_delay) / 2 / 1000000 * config->speed_of_sound;
}

void Calibration::average_dists_and_get_coords(){
    for (int node_i = 0; node_i < N_ALL_NODES; node_i++){
        for (int node_j = 0; node_j < N_ALL_NODES; node_j++){
            float total_dist = 0;
            for (int cycle = 0; cycle < n_cycles_cmd; cycle++){
                total_dist += cal_data.cycle_results[cycle].dists[node_i].dist[node_j];
            }
            total_dist /= n_cycles_cmd;
            cal_data.averaged_distances[node_i].dist[node_j] = total_dist;
        }
    }

    // Now cal_data.averaged_distances is pooulated with distances_t[4]
    // which contains the averaged distances between each node and each other node.
    // Convert to coordinates_t[4] and save to cal_data.coords:
    coord_3d_t coords[N_ALL_NODES];

    if (!has_depths){
        for (int i = 0; i < 3; i++){
            depths[i+1] = depths[0];
        }
    }
    calculate_coordinates(coords, cal_data.averaged_distances, depths);
}

void Calibration::resolve_cycle_dists(){
    cycle_result_t* cycle_result = &cal_data.cycle_results[cal_data.cycle_count];
    cycle_node_result_t* measured_times = cycle_result->node_results;
// void Calibration::trip_times_to_calibration_dists(distances_t dists[N_ALL_NODES], cycle_node_result_t measured_times[3]){
    float dist_0_1 = trip_time_to_dist(measured_times[0].cycle_time_first_response);
    float dist_0_2 = trip_time_to_dist(measured_times[1].cycle_time_first_response);
    float dist_0_3 = trip_time_to_dist(measured_times[2].cycle_time_first_response);

    float dist_1_2 = trip_time_to_dist(measured_times[0].cycle_time_second_response) * 2. + dist_0_1 - dist_0_2;
    float dist_2_3 = trip_time_to_dist(measured_times[1].cycle_time_second_response) * 2. + dist_0_2 - dist_0_3;
    float dist_1_3 = trip_time_to_dist(measured_times[2].cycle_time_second_response) * 2. + dist_0_3 - dist_0_1;

    cycle_result->dists[0] = {{0.,       dist_0_1, dist_0_2, dist_0_3}};
    cycle_result->dists[1] = {{dist_0_1, 0.,       dist_1_2, dist_1_3}};
    cycle_result->dists[2] = {{dist_0_2, dist_1_2, 0.,       dist_2_3}};
    cycle_result->dists[3] = {{dist_0_3, dist_1_3, dist_2_3, 0.0}};
}


// !!!! We can bring this back if we need it...
// void Calibration::manual_calibration_coords(const String token, const float depth){
//     int pos2 = 1;
//     for (uint8_t i = 0; i < N_ALL_NODES; ++i){
//         int next_pos2 = token.indexOf(';', pos2);
//         if (next_pos2 == -1) {
//             next_pos2 = token.length();
//         }
//         cal_data.node_coords_3d[i] = get_coord_from_string(token.substring(pos2, next_pos2));
//         pos2 = next_pos2 + 1;
//     }
//     // og_depth = (float)get_depth_mm() / 1000.0;

//     String new_coord_string = "Updated coordinates: [";
//     for (uint8_t i = 0; i < N_ALL_NODES; ++i){
//         new_coord_string += "[" + String(cal_data.node_coords_3d[i].x, 2) + ", " + 
//                             String(cal_data.node_coords_3d[i].y, 2) + ", " + 
//                             String(cal_data.node_coords_3d[i].z, 2) + "]";
//     }
// }


void Calibration::set_manual_depths(const String depths_str){
    // expect format like "0.0,0.0,0.0"
    float vals[3];
    bool parse_error = false;
    for (int i = 0; i < 3; i++){
        int next_comma = depths_str.indexOf(',', i);
        if (next_comma == -1){
            next_comma = depths_str.length();
        }
        vals[i] = depths_str.substring(i, next_comma).toFloat();
        i = next_comma;
    }

    if (!parse_error){
        depths[1] = vals[0];
        depths[2] = vals[1];
        depths[3] = vals[2];
        has_depths = true;
    }
    else{
        has_depths = false;
        client->print("ERROR: could not parse depths string: " + depths_str + "\n");
    }
}

void Calibration::update_rov_depth_50ms(){
    depths[0] = (float)get_depth_mm_50ms() / 1000.0;
}