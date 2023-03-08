#include <iostream>
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
    threeway_state = 0;
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
    threeway_state = 0;
    cal_data.cycle_count = 0;
        
    ts_delay_threeway_until = micros();
    ts_listen_start = 0; // doesnt need reset due to being set before use anyway, but for explicitness reset to 0
    ts_listen_timeout = 0; // doesnt need reset due to being set before use anyway, but for explicitness reset to 0
    ts_threeway_start = 0; // doesnt need reset due to being set before use anyway, but for explicitness reset to 0

    for (uint16_t i=0; i<MAX_CALIBRATION_CYCLES; i++){
        for (uint8_t j=0; j<3; j++){
            // cal_data.cycle_results[i].n_retries = 0;
            cal_data.cycle_results[i].node_results[j].first_response = 0;
            cal_data.cycle_results[i].node_results[j].second_response = 0;
        }
        for (uint8_t j=0; j<N_ALL_NODES; j++){
            for (uint8_t k=0; k<N_ALL_NODES; k++){
                cal_data.cycle_results[i].dists[j].dist[k] = 0;
            }
        }
    }

    return &cal_data;
}

uint64_t Calibration::yell_to_listen_offset(){
    return config->micros_send_duration + config->period - MICROS_TO_LISTEN_BEFORE_END_OF_PERIOD;
}
uint64_t Calibration::prev_listen_finish_to_listen_offset(){
    return config->period - MICROS_TO_LISTEN_BEFORE_END_OF_PERIOD;
}

uint64_t Calibration::listen_start_to_timeout_offset(){
    return config->period;
}

uint64_t Calibration::listen_to_yell_offset(){
    return config->period;
}


coord_3d_t Calibration::get_coord_from_string(String str){
    int idx_delimiter_1 = str.indexOf(',');
    int idx_delimiter_2 = str.indexOf(',', idx_delimiter_1 + 1);
    return coord_3d_t{
        float(atof(str.substring(0, idx_delimiter_1).c_str())),
        float(atof(str.substring(idx_delimiter_1 + 1, idx_delimiter_2).c_str())),
        float(atof(str.substring(idx_delimiter_2 + 1, str.length()).c_str())),
    };
}

void Calibration::init_threeway_start_yell_5ms(){
    cycle_result_t* cycle_result = &cal_data.cycle_results[cal_data.cycle_count];
    threeway_result_t* threeway_result = &cycle_result->node_results[freq_idx_to_node_n(calibrating_node_135)];
    threeway_result->first_response = 0;
    threeway_result->second_response = 0;
    ts_threeway_start = talker->begin_yell_5ms(calibrating_node_135);
    ts_listen_start = ts_threeway_start + yell_to_listen_offset();
    ts_listen_timeout = 0; // doesnt need reset due to being set before use anyway, but for explicitness reset to 0
    threeway_state = THREEWAY_STATE_YELL;
}

bool Calibration::calibration_in_progress_tick(){
    // start_yell_listen_x2_0123 is a state machine.
    //  ->  0= start, 1= send, 2= listen first reply, 3= listen second reply
    if (threeway_state == THREEWAY_STATE_INIT){ // starting node
        if (micros() < ts_delay_threeway_until){
            return false; // not ready to start yet
        }
        init_threeway_start_yell_5ms();
        threeway_state = THREEWAY_STATE_YELL; // gets set in init_node_cycle, but for explicitness
        return false; // only return true when done calibration
    }
    else if (threeway_state == THREEWAY_STATE_YELL){ // yelling
        bool yell_completed = talker->yell();
        if (yell_completed){ 
            // setup for listening mode
            threeway_state = THREEWAY_STATE_LISTEN_1;
            switch_relay_to_receive_6ms();
            ts_listen_timeout = ts_listen_start + listen_start_to_timeout_offset();
            update_rov_depth_50ms(); // blocking stuff
            listener->begin(ts_listen_start);
            listener->start_adc_timer();
        }
        return false; // only return true when done calibration
    }
    else if (threeway_state == THREEWAY_STATE_LISTEN_1 || threeway_state == THREEWAY_STATE_LISTEN_2){ // listening
        // perform listening tick. if calibration done, returns true which we propagate up
        if (listen_return_is_calibration_done()){
            is_calibrated = true;
            average_dists_and_get_coords();
            return true; // return true = done calibration
        }
    }
    else{
        client->print("ERROR: Bad code brought start_send_listen_x2_0123_0123 out of range to "+String(threeway_state)+"\n");
    }
    return false; // only return true when done calibration
}

bool Calibration::listen_return_is_calibration_done(){ 
    uint8_t node_id_to_listen_for = (calibrating_node_135 + 1) % 6;
    listener_output_t result = listener->hb();

    bool should_increment_node = handle_result_and_return_should_increment_node(&result, node_id_to_listen_for);
    if (should_increment_node){

        bool should_increment_cycle = increment_node_and_return_should_increment_cycle();
        if (should_increment_cycle){

            resolve_cycle_dists(); // convert times to dists

            bool calibration_done = increment_cycle_return_is_calibration_done();
            if (calibration_done){
                return true; // done calibration, return true
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
    threeway_state = 0;
    if (calibrating_node_135 == 5){
        // cal_data.cycle_count += 1;
        calibrating_node_135 = 1;
        return true;
    }
    calibrating_node_135 += 2;
    return false;
}

bool Calibration::handle_result_and_return_should_increment_node(listener_output_t* result, uint8_t node_id_to_listen_for){
    bool retry_due_to_fail = !check_timeout_and_freq(result, node_id_to_listen_for);
    if (retry_due_to_fail){
        cal_data.retries[node_id_to_listen_for]++;
        threeway_state = THREEWAY_STATE_INIT;
        listener->end_adc_timer();
        ts_delay_threeway_until = micros() + listen_to_yell_offset();
        switch_relay_to_send_5ms();
        return false; // return false = dont increment node, retry it instead
    }

    if (!result->finished){ // return false = dont increment node (im not ready yet)
        return false;
    }

    // Finished listening and received the expected frequency

    cycle_result_t* cycle_result = &cal_data.cycle_results[cal_data.cycle_count];
    threeway_result_t* threeway_result = &cycle_result->node_results[freq_idx_to_node_n(calibrating_node_135)];

    uint64_t elapsed_since_threeway_start = result->ts_peak - ts_threeway_start;
    if (threeway_state == THREEWAY_STATE_LISTEN_1){
        threeway_result->first_response = elapsed_since_threeway_start;
        ts_listen_start = result->ts_peak + prev_listen_finish_to_listen_offset();
        ts_listen_timeout = ts_listen_start + listen_start_to_timeout_offset();
        listener->begin(ts_listen_start);
        threeway_state = THREEWAY_STATE_LISTEN_2;
        return false; // dont increment node
    }
    else if (threeway_state == THREEWAY_STATE_LISTEN_2){
        threeway_result->second_response = elapsed_since_threeway_start;
        threeway_state = THREEWAY_STATE_INIT; // for explicitness       (set in increment_node)
        ts_listen_start = 0; //           for explicitness, unnecessary reset to 0 (set in init_threeway_start_yell_5ms)
        ts_listen_timeout = 0; //         for explicitness, unnecessary reset to 0 (set in init_threeway_start_yell_5ms)
        listener->end_adc_timer();
        switch_relay_to_send_5ms();
        ts_delay_threeway_until = result->ts_peak + listen_to_yell_offset();
        return true; // ready to increment to the next node (next threeway)
    }
    else {
        client->print("ERROR: State machine fuckup... how can threeway_state be"+String(threeway_state)+"?\n");
        return false; // if there is state machine fuckup, keep retrying till a human notices i guess
    }
    return false; // if there is bad code, keep retrying till a human notices i guess
}

bool Calibration::check_timeout_and_freq(listener_output_t* result, uint8_t node_id_to_listen_for){
    if (!result->finished){
        if (micros() >= ts_listen_timeout){
            char msg[80];
            // TODO: printing uint64s sucks listen_start_to_timeout_offset is a uint64
            sprintf(msg, "ERROR: timeout after %umicros listening for id %u\n",listen_start_to_timeout_offset(), node_id_to_listen_for);
            client->print(String(msg));
            return false;
        }
        return true; // if not finished and not timed out, all ok
    }
    if (result->idx_identified_freq != node_id_to_listen_for){
        char msg[80];
        sprintf(msg, "ERROR: expected to hear id %u, but heard id %u\n", node_id_to_listen_for, result->idx_identified_freq);
        client->print(String(msg));
        return false;
    }
    return true; // all ok
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
    // Convert to coordinates_t[4] and save to cal_data.node_coords_3d:

    if (!has_depths){
        for (int i = 0; i < 3; i++){
            depths[i+1] = depths[0];
        }
    }
    calculate_coordinates(cal_data.node_coords_3d, cal_data.averaged_distances, depths);
}

void Calibration::resolve_cycle_dists(){
    cycle_result_t* cycle_result = &cal_data.cycle_results[cal_data.cycle_count];
    threeway_result_t* measured_times = cycle_result->node_results;
// void Calibration::trip_times_to_calibration_dists(distances_t dists[N_ALL_NODES], cycle_node_result_t measured_times[3]){
    float dist_0_1 = trip_time_to_dist(measured_times[0].first_response);
    float dist_0_2 = trip_time_to_dist(measured_times[1].first_response);
    float dist_0_3 = trip_time_to_dist(measured_times[2].first_response);

    float dist_1_2 = trip_time_to_dist(measured_times[0].second_response) * 2. + dist_0_1 - dist_0_2;
    float dist_2_3 = trip_time_to_dist(measured_times[1].second_response) * 2. + dist_0_2 - dist_0_3;
    float dist_1_3 = trip_time_to_dist(measured_times[2].second_response) * 2. + dist_0_3 - dist_0_1;

    cycle_result->dists[0] = {{0.,       dist_0_1, dist_0_2, dist_0_3}};
    cycle_result->dists[1] = {{dist_0_1, 0.,       dist_1_2, dist_1_3}};
    cycle_result->dists[2] = {{dist_0_2, dist_1_2, 0.,       dist_2_3}};
    cycle_result->dists[3] = {{dist_0_3, dist_1_3, dist_2_3, 0.0}};
}


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


// !!!! We can bring this back if we need it...
void Calibration::manual_calibration_coords(const String token){
    int pos2 = 1;
    for (uint8_t i = 0; i < N_ALL_NODES; ++i){
        int next_pos2 = token.indexOf(';', pos2);
        if (next_pos2 == -1) {
            next_pos2 = token.length();
        }
        cal_data.node_coords_3d[i] = get_coord_from_string(token.substring(pos2, next_pos2));
        pos2 = next_pos2 + 1;
    }
    // og_depth = (float)get_depth_mm() / 1000.0;

    String new_coord_string = "Updated coordinates: [";
    for (uint8_t i = 0; i < N_ALL_NODES; ++i){
        new_coord_string += "[" + String(cal_data.node_coords_3d[i].x, 2) + ", " + 
                            String(cal_data.node_coords_3d[i].y, 2) + ", " + 
                            String(cal_data.node_coords_3d[i].z, 2) + "]";
    }
    is_calibrated = true;
}