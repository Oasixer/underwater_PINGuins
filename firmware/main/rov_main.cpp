#include "rov_main.h"
#include "adc_isr.h"
#include "dac_driver.h"
#include "relay.h"
#include "fourier.h"
#include "printing.h"
#include "tcp_client.h"
#include "utils.h"
#include "MS5839.h"
#include "multilateration.h"

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

// buffer to store trip times
#define MAX_N_TRIPS 500

RovMain::RovMain(config_t* config, Listener* listener, TcpClient* client){
    frequency_magnitudes = get_frequency_magnitudes();
    last_reading = get_last_reading();
    this->config = config;
    this->listener = listener;
    this->client = client;

    for (uint8_t i = 0; i<3; ++i){
        trip_times_round_robin[i] = 0;
    }
    n_round_robins_done = 0;
}

void RovMain::setup(){
    pinMode(NO_LEAK_PIN, INPUT);
    dac_setup(DAC_PIN, DAC_CLR_PIN, HV_ENABLE_PIN);
    
    setup_relay();
    switch_relay_to_send();
    client->print("Setup relay\n");

    adc_setup();
    client->print("Setup adc and timer\n");

    fourier_initialize(config->fourier_window_size);
    client->print("Setup fourier\n");

    MS5839_init();
    client->print("Initialized pressure sensor\n");

    delay(50);
}

void RovMain::shutdown(){
    adc_timer.end();
}

void RovMain::reset_send_receive(){
    adc_timer.end();

    for(uint8_t i = 0; i<3; i++){
        trip_times_round_robin[i] = 0;
    }

    ts_start_talking = micros() + config->period;
    ts_response_timeout = ts_start_talking + config->response_timeout_duration;

    curr_freq_idx = 1;
    frequency_to_send = FREQUENCIES[curr_freq_idx];
    n_round_robins_done++;
    is_currently_receiving = false;
    switch_relay_to_send();
}

void RovMain::send_mode_hb(){
    if (micros() >= ts_start_talking){ // send data
        if (micros() - ts_start_talking < config->micros_send_duration){ // keep sending
            dac_set_analog_float(sinf(2 * M_PI * frequency_to_send / 1000000 * (float)(micros() % (1000000 / frequency_to_send))));

        } else { // finished beep
            listener->begin(micros() + config->period - MICROS_TO_LISTEN_BEFORE_END_OF_PERIOD);
            curr_depth = (float)get_depth_mm() / 1000.0;
            is_currently_receiving = true; // switch to receiving
            switch_relay_to_receive();
            adc_timer.begin(adc_timer_callback, ADC_PERIOD);
        }
    }
}

coord_3d_t get_coord_from_string(String str){
    int idx_delimiter_1 = str.indexOf(',');
    int idx_delimiter_2 = str.indexOf(',', idx_delimiter_1 + 1);
    return coord_3d_t{
        float(atof(str.substring(0, idx_delimiter_1).c_str())),
        float(atof(str.substring(idx_delimiter_1 + 1, idx_delimiter_2).c_str())),
        float(atof(str.substring(idx_delimiter_2 + 1, str.length()).c_str())),
    };
}

bool RovMain::loop(){
    if(digitalRead(NO_LEAK_PIN) == THERE_IS_A_LEAK){
        digitalWrite(HV_ENABLE_PIN, LOW);    // turn off high voltage
        switch_relay_to_receive();    // switch to receive mode
        adc_timer.end(); // turn off ADC timer so that we only send Leak Detect messages
        // client->print("LEAK DETECTED\n");
        client->send_leak_detected_panic_message();
    } else {
        String message= "";
        //Serial.println("adc main()");

        if (client->has_cmd_available()){
            Serial.println("grab cmd");
            message = client->get_incoming_cmd();
            Serial.println("got mnsg");
        }
        else if (Serial.available() > 0){
            message = Serial.readStringUntil(message_terminator);
        }
        if (message.length() > 0) {
            
            // Split the message into tokens using the delimiter
            int pos = 0;
            while (pos < message.length()) {
                int next_pos = message.indexOf(message_delimiter, pos);
                if (next_pos == -1) {
                    next_pos = message.length();
                }
                String token = message.substring(pos, next_pos);
                pos = next_pos + 1;

                // Extract the field value based on the token prefix
                if (token.startsWith("h")){    // just a ping
                    client->print("hi\n");

                } else if (token.startsWith("x")){ // stop everything and go to default state;
                    adc_timer.end();
                    is_transmit_continuously = false;
                    is_currently_receiving = false;
                    n_talks_done = 0;
                    n_talks_command = 0;
                    n_round_robins_command = 0;
                    n_round_robins_done = 0;
                    curr_freq_idx = 1;
                    switch_relay_to_send();
                    client->print("Stopped\n");

                } else if (token.startsWith("b")) { // change marco polo time delay
                    config -> marco_polo_time_delay = atof(token.substring(1).c_str());
                    client->print("Changed marco polo time delay to " + String(config->speed_of_sound) + "us\n");
                }
                else if(token == "B_STATIONARY"){
                    client->print("Becoming stationary");
                    return true;
                } else if (token.startsWith("g")) { // talk for x times
                    n_talks_command = (uint16_t)token.substring(1).toInt();
                    ts_start_talking = micros();    // start talking now
                    ts_response_timeout = ts_start_talking + config->response_timeout_duration;
                    client->print("Gonna talk " + String(n_talks_command) + " times\n");

                } else if (token.startsWith("R")) { // find position
                    n_round_robins_command = (uint16_t)token.substring(1).toInt();
                    ts_start_talking = micros();    // start talking now
                    ts_response_timeout = ts_start_talking + config->response_timeout_duration;
                    client->print("Gonna find position " + String(n_round_robins_command) + " times\n");
            
                } else if (token.startsWith("f")) {    // change my frequency
                    frequency_to_send = (uint16_t)token.substring(1).toInt();
                    client->print("Changed frequency to send to " + String(frequency_to_send) + "Hz\n");

                } else if (token.startsWith("N")) {    // change window size
                    config->fourier_window_size = (uint16_t)token.substring(1).toInt();
                    config->micros_send_duration = ADC_PERIOD * config->fourier_window_size;
                    client->print("Changed window size to " + String(config->fourier_window_size) + "\n");
                
                } else if (token.startsWith("t")) {    // change threshold
                    config->dft_threshold = (uint32_t)token.substring(1).toInt();
                    client->print("Changed DFT threshold to " + String(config->dft_threshold) + "\n");

                } else if (token.startsWith("c")){ // transmit continuously
                    uint32_t millis_to_continuously_transmit = (uint32_t)token.substring(1).toInt();
                    client->print("Will transmit continuously for " + String(millis_to_continuously_transmit) + "ms\n");
                    is_transmit_continuously = true;
                    ts_stop_continuous_transmission = micros() + millis_to_continuously_transmit*1000;
                
                } else if (token.startsWith("r")) { // use rising edge 
                    uint8_t use_rising_edge = (uint8_t)token.substring(1).toInt();
                    config->use_rising_edge = use_rising_edge > 0;
                    client->print("Use rising edge set to " + String(config->use_rising_edge) + "\n");

                } else if (token.startsWith("I")) { // change identificiation type
                    uint8_t integrate_freq_domain = (uint8_t)token.substring(1).toInt();
                    config->integrate_freq_domain = integrate_freq_domain > 0;
                    client->print("Integrate freq domain set to " + String(config->integrate_freq_domain) + "\n");

                } else if (token.startsWith("T")) { // change period
                    uint32_t period_ms = (uint32_t)token.substring(1).toInt();
                    config->period = period_ms * 1000;
                    config->response_timeout_duration = 2 * config->period;
                    client->print("Changed period to " + String(period_ms) + "ms\n");
                                        
                } else if (token.startsWith("v")) { // change speed of sound
                    config->speed_of_sound = atof(token.substring(1).c_str());
                    client->print("Changed speed of sound to " + String(config->speed_of_sound) + "m/s\n");

                } else if (token.startsWith("b")) { // change marco polo time delay
                    config->marco_polo_time_delay = atof(token.substring(1).c_str());
                    client->print("Changed marco polo time delay to " + String(config->speed_of_sound) + "us\n");

                } else if (token.startsWith("d")) { // change duration to find peak
                    config->duration_to_find_peak = (uint16_t)token.substring(1).toInt();
                    client->print("Changed duration to find peak to " + String(config->duration_to_find_peak) + "us\n");

                } else if (token.startsWith("C")) { // input of node coordinates
                    int pos2 = 1;
                    for (uint8_t i = 0; i < N_ALL_NODES; ++i){
                        int next_pos2 = token.indexOf(';', pos2);
                        if (next_pos2 == -1) {
                            next_pos2 = token.length();
                        }
                        node_coords_3d[i] = get_coord_from_string(token.substring(pos2, next_pos2));
                        pos2 = next_pos2 + 1;
                    }
                    og_depth = (float)get_depth_mm() / 1000.0;

                    String new_coord_string = "Updated coordinates: [";
                    for (uint8_t i = 0; i < N_ALL_NODES; ++i){
                        new_coord_string += "[" + String(node_coords_3d[i].x, 2) + ", " + 
                                            String(node_coords_3d[i].y, 2) + ", " + 
                                            String(node_coords_3d[i].z, 2) + "]";
                    }
                    client->print(new_coord_string + "]\nSet og depth to " + String(og_depth, 4) + "m\n");
                
                }
            }
        }

        if (is_transmit_continuously) {    // if the command is to transmit continuously
            if (micros() < ts_stop_continuous_transmission) { // should still transmit
                dac_set_analog_float(sinf(2 * M_PI * frequency_to_send / 1000000 * (float)(micros() % (1000000 / frequency_to_send))));
            } else { // stop transmitting
                client->print("Finished continuous transmission\n");
                is_transmit_continuously = false;
            }

        } else if (n_talks_done < n_talks_command){ // if still have to send receive more
            if (is_currently_receiving){
                listener_output_t listener_data = listener->hb();
                receive_mode_hb_single_freq(listener_data);
            } else {    // sending
                send_mode_hb();
            }

        } else if (n_round_robins_done < n_round_robins_command){ // find position
            if (is_currently_receiving){
                listener_output_t listener_data = listener->hb();
                round_robin_receive_mode_hb(listener_data);
            }
            else {  // sending
                send_mode_hb();
            }
        }
    }
    return false; // only return true if becoming stationary
}

void RovMain::receive_mode_hb_single_freq(listener_output_t &listener_data){
    bool finished = false;
    uint64_t ts_peak = -1;
    if (listener_data.finished){
        finished = true;
        ts_peak = listener_data.ts_peak;
    } else {
        if (micros() >= ts_response_timeout){
            client->print("ERROR timeout\n");
            finished = true;
        }
    }

    if (finished){
        adc_timer.end();
        uint64_t trip_time = ts_peak - ts_start_talking;
        trip_times_single_freq[n_talks_done] = trip_time;
        float dist_estimate = trip_time_to_dist(trip_time);
        client->print("Trip time: " + uint64ToString(trip_time) + "us. Measured Distance: " + 
                     String(dist_estimate, 3) + "m\n");

        ts_start_talking = micros() + config->period;
        ts_response_timeout = ts_start_talking + config->response_timeout_duration;
        // client->print("Ts_start talking: " + uint64ToString(ts_start_talking) + "\n");

        is_currently_receiving = false;
        switch_relay_to_send();

        ++n_talks_done;
        if (n_talks_done >= n_talks_command){  // did all the commanded send receives
            client->print("Finished send receives " + String(n_talks_done) + " times. Measured distances (m) are:\n");
            String msg = "[";
            for (uint16_t i = 0; i < n_talks_done; ++i){
                msg += String(trip_time_to_dist(trip_times_single_freq[i]), 3); 
                msg += "; ";
                if (i % 10 == 9){
                    client->print(msg + "\n");
                    msg = "";
                }
            }
            client->print(msg + "]\nTrip times are: \n");
            msg = "[";
            for (uint16_t i = 0; i < n_talks_done; ++i){
                msg += uint64ToString(trip_times_single_freq[i]); 
                msg += "; ";
                if (i % 10 == 9){
                    client->print(msg + "\n");
                    msg = "";
                }
            }
            client->print(msg + "]\n\n");
            n_talks_command = 0;
            n_talks_done = 0;
        } else if (n_talks_done > MAX_N_TRIPS){
            n_talks_done = 0;
        }
    }
}

void RovMain::round_robin_receive_mode_hb(listener_output_t &listener_data){
    if (listener_data.finished){
        client->print("Found freq: " + String(curr_freq_idx) + "\n");
        if (listener_data.idx_identified_freq == curr_freq_idx){
            adc_timer.end();

            uint64_t trip_micros = listener_data.ts_peak - ts_start_talking;
            trip_times_round_robin[curr_freq_idx-1] = trip_micros;

            // float distance = micros_to_meters(trip_micros);
            ts_start_talking = listener_data.ts_peak + config->period;
            ts_response_timeout = ts_start_talking + config->response_timeout_duration;

            if (++curr_freq_idx >= 4) { // done this round robin
                float dists_3d[N_ALL_NODES] = {
                    0.0, // distance from rov to rov
                    trip_time_to_dist(trip_times_round_robin[0]), // dist to node 1
                    trip_time_to_dist(trip_times_round_robin[1]), // dist to node 2
                    trip_time_to_dist(trip_times_round_robin[2]), // dist to node 3
                };
                coord_3d_t position_estimate = multilaterate(node_coords_3d, dists_3d, curr_depth, og_depth);
                client->print("Distances: [0.0, " + String(dists_3d[1], 2) + ", " + 
                    String(dists_3d[2], 2) + ", " + String(dists_3d[3], 2) + "], Depth: " + 
                    String(curr_depth, 3) + ", Estimate: [" + String(position_estimate.x, 2) + 
                    ", " + String(position_estimate.y, 2) + ", " + String(position_estimate.z, 2) + "]\n");

                // reset for next time
                curr_freq_idx = 1;
                ++n_round_robins_done;
            }

            frequency_to_send = FREQUENCIES[curr_freq_idx];
            switch_relay_to_send();
            is_currently_receiving = false;
        }
        else{
            client->print("ERROR: Unexpected frequency: " + String(listener_data.idx_identified_freq) +"| expected freq #"+String(curr_freq_idx)+"\n");
            reset_send_receive();
        }
    }
    else{
        if (micros() >= ts_response_timeout){
            client->print("ERROR: timeout while expecting freq #"+String(curr_freq_idx)+"\n");
            reset_send_receive();
        }
    }
}

float RovMain::trip_time_to_dist(uint64_t trip_time){
    return ((float)trip_time - config->marco_polo_time_delay) / 2 / 1000000 * config->speed_of_sound;
}