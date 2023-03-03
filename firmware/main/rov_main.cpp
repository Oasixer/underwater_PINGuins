#include "rov_main.h"
#include "adc_isr.h"
#include "dac_driver.h"
#include "relay.h"
#include "fourier.h"
#include "printing.h"
#include "tcp_client.h"
#include "utils.h"

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

// buffer to store trip times
#define MAX_N_TRIPS 500

RovMain::RovMain(config_t* config, Listener* listener){
    frequency_magnitudes = get_frequency_magnitudes();
    last_reading = get_last_reading();
    this->config = config;
    this->listener = listener;

    for (uint16_t i = 0; i < MAX_N_TRIPS; ++i){
        for (uint8_t j = 0; j<3; ++j){
            trip_times[i][j] = 0;
        }
    }
    n_round_robins_done = 0;
}

void RovMain::setup(TcpClient* client){
    this->client = client;
    // config->my_frequency = 25000;
    config->my_frequency_idx = F_LOW;

    pinMode(NO_LEAK_PIN, INPUT);
    dac_setup(DAC_PIN, DAC_CLR_PIN, HV_ENABLE_PIN);
    
    setup_relay();
    switch_relay_to_send();
    client->print("Setup relay\n");

    adc_setup();
    client->print("Setup adc and timer\n");

    fourier_initialize(config->fourier_window_size);
    client->print("Setup fourier\n");
}

// void RovMain::detect_frequencies() {
//     float magnitudes[N_FREQUENCIES];

//     noInterrupts();
//     for (uint8_t i = 0; i < N_FREQUENCIES; ++i) {
//         magnitudes[i] = frequency_magnitudes[i];
//     }
//     interrupts();

//     for (uint8_t i = 0; i < N_FREQUENCIES; ++i) {
//         if (magnitudes[i] > config->dft_threshold) {
//             // initialize peak finding variables and switch to peak finding state
//             curr_max_magnitude = magnitudes[i];
//             ts_peak = micros();
//             ts_peak_finding_timeout = micros() + config->micros_to_find_peak;
//             idx_freq_detected = i;
//             is_peak_finding = true;

//             client->print("After " + uint64ToString(micros() - ts_start_listening) + 
//                 "us of listening, detected freq " + String(FREQUENCIES[i]) + 
//                 "Hz, with magnitude " + String(curr_max_magnitude, 0) + ", with magnitudes: [" + 
//                 String(magnitudes[0], 0) + ", " + String(magnitudes[1], 0) + ", " + 
//                 String(magnitudes[2], 0) + ", " + String(magnitudes[3], 0) + ", " + 
//                 String(magnitudes[4], 0) + ", " + String(magnitudes[5], 0) + "]\n");

//             return;    // this should only be true for one frequency. there's no point in checking the rest
//         }
//     }
// }

// void RovMain::reset_send_receive(){
//     uint64_t trip_time = ts_peak - ts_start_talking;
//     trip_times[n_talks_done] = trip_time;
//     measured_distances[n_talks_done] = ((float)trip_time - MARCO_POLO_TIME_DELAY_US) / 2 / 1000000 * SPEED_SOUND; // store for later
//     client->print("Trip time: " + uint64ToString(trip_time) + "us. Measured Distance: " + 
//     String(measured_distances[n_talks_done], 3) + "m\n");

//     curr_max_magnitude = 0;    // reset to zero for next time
//     is_peak_finding = false;    // start next time not in peak finding state
//     is_currently_receiving = false; // switch to sending state for next time
//     adc_timer.end();
//     switch_relay_to_send();

//     if (++n_talks_done < n_talks_command){    // still have to send receive more
//         ts_start_talking = micros() + INACTIVE_DURATION_BEFORE_TALKING;
//         ts_response_timeout = ts_start_talking + config->response_timeout_duration;

//     } else {    // did all the commanded send receives
//         client->print("Finished send receives " + String(n_talks_done) + " times. Measured distances (m) are:\n");
//         String msg = "[";
//         for (uint16_t i = 0; i < n_talks_done; ++i){
//             msg += String(measured_distances[i], 3); 
//             msg += "; ";
//             if (i % 10 == 9){
//                 client->print(msg + "\n");
//                 msg = "";
//             }
//         }
//         client->print(msg + "]\nTrip times are: \n");
//         msg = "[";
//         for (uint16_t i = 0; i < n_talks_done; ++i){
//             msg += uint64ToString(trip_times[i]); 
//             msg += "; ";
//             if (i % 10 == 9){
//                 client->print(msg + "\n");
//                 msg = "";
//             }
//         }
//         client->print(msg + "]\n\n");
//         n_talks_command = 0;
//         n_talks_done = 0;
//     }
// }

// void RovMain::rov_peak_finding(){
//     if (config->use_rising_edge || micros() >= ts_peak_finding_timeout){    // finished peak finding
//         reset_send_receive();
//     } else {
//         if (frequency_magnitudes[idx_freq_detected] > curr_max_magnitude){
//             curr_max_magnitude = frequency_magnitudes[idx_freq_detected];
//             ts_peak = micros();
//         }
//     }
// }

// void RovMain::rov_receive_mode_hb(){
//     if (micros() >= ts_response_timeout){ // assume wont get a response
//         ts_peak = -1;    // indicate that was not able to find peak
//         client->print("TIMEOUT didn't hear a response\n");
//         reset_send_receive();
//     } else {
//         if (micros() >= ts_start_listening){    // if not in inactive period
//             if (is_peak_finding) {
//                 rov_peak_finding();
//             } else {
//                 detect_frequencies();
//             }
//         }
//     }
// }

void RovMain::reset_send_receive(){
    adc_timer.end();

    for(uint8_t i = 0; i<3; i++){
        trip_times[n_round_robins_done][i] = 0;
    }

    curr_expected_freq_idx = 1;
    n_round_robins_done++;
    is_currently_receiving = false;
    switch_relay_to_send();
}

void RovMain::send_mode_hb(){
    if (micros() >= ts_start_talking){ // send data
        if (micros() - ts_start_talking < config->micros_send_duration){ // keep sending
            const uint16_t my_freq = get_freq(config->my_frequency_idx);
            dac_set_analog_float(sinf(2 * M_PI * my_freq / 1000000 * (float)(micros() % (1000000 / my_freq))));

        } else { // finished beep
            ts_start_listening = micros() + config->period - MICROS_TO_LISTEN_BEFORE_END_OF_PERIOD;
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
        atof(str.substring(0, idx_delimiter_1).c_str()),
        atof(str.substring(idx_delimiter_1 + 1, idx_delimiter_2).c_str()),
        atof(str.substring(idx_delimiter_2 + 1, str.length()).c_str()),
    };
}

void RovMain::loop(){
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
                    

                } else if (token.startsWith("s")){ // stop everything and go to default state;
                    is_transmit_continuously = false;
                    switch_relay_to_send();
                    adc_timer.end();
                    n_talks_done = 0;
                    n_talks_command = 0;
                    is_currently_receiving = false;
                    is_peak_finding = false;
                    client->print("Stopped\n");

                } else if (token.startsWith("g")) { // talk for x times
                    n_talks_command = (uint16_t)token.substring(1).toInt();
                    ts_start_talking = micros();    // start talking now
                    client->print("Gonna talk " + String(n_talks_command) + " times\n");
                    ts_response_timeout = ts_start_talking + config->response_timeout_duration;
            
                } else if (token.startsWith("f")) {    // change my frequency
                    // config->my_frequency = (uint16_t)token.substring(1).toInt();
                    config->my_frequency_idx = (uint16_t)token.substring(1).toInt();
                    client->print("Changed my frequency to " + String(config->my_frequency_idx) + "\n");

                } else if (token.startsWith("N")) {    // change window size
                    config->fourier_window_size = (uint16_t)token.substring(1).toInt();
                    // config->micros_to_find_peak = ADC_PERIOD * config->fourier_window_size;
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

                } else if (token.startsWith("C")) { // input of node coordinates
                    coord_3d_t coordinates[3]; // TODO @ahmad i added this so it compiles
                    int pos2 = 1;
                    for (uint8_t i = 0; i < N_ALL_NODES; ++i){
                        int next_pos2 = token.indexOf(';', pos2);
                        if (next_pos2 == -1) {
                            next_pos2 = token.length();
                        }
                        coordinates[i] = get_coord_from_string(token.substring(pos2, next_pos2));
                        pos2 = next_pos2 + 1;
                    }
                    String new_coord_string = "Updated coordinates: [";
                    for (uint8_t i = 0; i < N_ALL_NODES; ++i){
                        new_coord_string += "[" + String(coordinates[i].x, 2) + ", " + String(coordinates[i].y, 2) + ", " + String(coordinates[i].z, 2) + "]";
                    }
                    client->print(new_coord_string + "]\n");

                } else if (token.startsWith("i")) { // change identificiation type
                    uint8_t integrate_freq_domain = (uint8_t)token.substring(1).toInt();
                    config->integrate_freq_domain = integrate_freq_domain > 0;
                    client->print("Integrate freq domain set to " + String(config->integrate_freq_domain) + "\n");

                } else if (token.startsWith("p")) { // change period
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
                }
            }
        }

        bool find_my_position = true; // TODO @ahmad, where we set this?

        if (is_transmit_continuously) {    // if the command is to transmit continuously
            if (micros() < ts_stop_continuous_transmission) { // should still transmit
                uint16_t my_freq = get_freq(config->my_frequency_idx);
                dac_set_analog_float(sinf(2 * M_PI * my_freq / 1000000 * (float)(micros() % (1000000 / my_freq))));
            } else { // stop transmitting
                client->print("Finished continuous transmission\n");
                is_transmit_continuously = false;
            }

        } else if (n_talks_done < n_talks_command){    // if still have to send receive more
            if (is_currently_receiving){
                // rov_receive_mode_hb();
            } else {    // sending
                send_mode_hb();
            }

        } else if (find_my_position){
            // TODO: check if no longer need to find my position

            if (is_currently_receiving){
                listener_output_t listener_data = listener->hb();
                receive_mode_hb(listener_data);
            }
            else {  // sending
                send_mode_hb();
            }
        }
    }
}

void RovMain::receive_mode_hb(listener_output_t &listener_data){
    if (listener_data.finished){
        if (listener_data.idx_identified_freq == curr_expected_freq_idx){
            adc_timer.end();

            uint64_t trip_micros = listener_data.ts_peak - ts_start_talking;
            trip_times[n_round_robins_done][curr_expected_freq_idx-1] = trip_micros;

            // float distance = micros_to_meters(trip_micros);
            ts_start_talking = listener_data.ts_peak + config->period;

            if (++curr_expected_freq_idx >= 4) { // n_nodes
                curr_expected_freq_idx = 1;
                // print all distances
                n_round_robins_done++;
            }

            switch_relay_to_send();
            is_currently_receiving = false;
        }
        else{
            client->print("ERROR: Unexpected frequency: " + String(listener_data.idx_identified_freq) +"| expected freq #"+String(curr_expected_freq_idx)+"\n");
            reset_send_receive();
        }
    }
    else{
        if (millis() >= ts_response_timeout){
            client->print("ERROR: timeout while expecting freq #"+String(curr_expected_freq_idx)+"\n");
            reset_send_receive();
        }
    }
}
