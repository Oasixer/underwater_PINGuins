#include "stationary_main.h"
#include "configurations.h"
#include "adc_isr.h"
#include "constants.h"
#include "dac_driver.h"
#include "relay.h"
#include "fourier.h"
#include "printing.h"
#include "utils.h"
#include "listener.h"
#include "tcp_client.h"
#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <IntervalTimer.h>


StationaryMain::StationaryMain(config_t* config, Listener* listener, TcpClient* client){
    frequency_magnitudes = get_frequency_magnitudes();
    largest_magnitudes = get_largest_magnitudes();
    last_reading = get_last_reading();
    fourier_counter = get_fourier_counter();
    this->config = config;
    this->listener= listener;
    this->client = client;
}

void StationaryMain::setup(){
    pinMode(NO_LEAK_PIN, INPUT);
    dac_setup(DAC_PIN, DAC_CLR_PIN, HV_ENABLE_PIN);
    
    setup_relay();
    switch_relay_to_receive_6ms();
    client->print("Setup relay\n");

    // adc_timer.begin(adc_timer_callback, ADC_PERIOD);
    listener->start_adc_timer();
    adc_setup();
    client->print("Setup adc and timer\n");

    fourier_initialize(config->fourier_window_size);
    client->print("Setup fourier\n");

    listener->begin(micros() + config->period);
}

void StationaryMain::shutdown(){
    listener->end_adc_timer();
}

void StationaryMain::send_mode_hb(){
    if (micros() >= ts_start_talking){
        reply_yell();
    }
}

void StationaryMain::reply_yell(){
    if (micros() - ts_start_talking < config->micros_send_duration){ // keep sending
        dac_set_analog_float(config->max_amplitude_factor * sinf(2 * M_PI * freq_to_send / 1000000 * (float)(micros() % (1000000 / freq_to_send))));
    } else { // finished beep
        ts_start_listening = micros() + config->period - MICROS_TO_LISTEN_BEFORE_END_OF_PERIOD;
        listener->begin(ts_start_listening);

        is_currently_receiving = true; // switch to receiving

        // client->print("sent for " + uint64ToString(micros() - ts_start_talking) + 
        //     "us. Finished sending at " + uint64ToString(micros()) + 
        //     ". Will start listening at " + uint64ToString(ts_start_talking) + "\n");
        switch_relay_to_receive_6ms();
        listener->start_adc_timer();
    }
}

bool StationaryMain::loop(){
    if(digitalRead(NO_LEAK_PIN) == THERE_IS_A_LEAK){
        digitalWrite(HV_ENABLE_PIN, LOW);    // turn off high voltage
        switch_relay_to_receive_6ms();    // switch to receive mode
        listener->end_adc_timer();
        client->send_leak_detected_panic_message();
    } else {
        String message= "";
        if (client->has_cmd_available()){
            message = client->get_incoming_cmd();
        }
        else if (Serial.available()){
            message = Serial.readStringUntil(message_terminator);
        }
        if (message.length() > 0){

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
                    client->print("hi: STATIONARY\n");
                } else if (token == "BECOME_ROV") { // change marco polo time delay
                    client->print("Becoming ROV\n");
                    return true;
                } else if (token.startsWith("x")){// stop trying to detect frequencies;
                    listen_for_call_and_respond = false;
                    freq_to_send = get_freq((my_stationary_idx - 1) * 2);
                    switch_relay_to_receive_6ms();
                    listener->start_adc_timer();
                    client->print("Stopped\n");
                } else if (token.startsWith("g")) { // start trying to detect frequencies
                    listen_for_call_and_respond = true;
                    switch_relay_to_receive_6ms();
                    // adc_timer.begin(adc_timer_callback, ADC_PERIOD);
                    listener->start_adc_timer();
                    is_peak_finding = false;
                    is_currently_receiving = true;
                    listener->begin(micros());
                    client->print("Started\n");

                } else if (token.startsWith("i")) {    // change my frequency
                    my_stationary_idx = (uint8_t)token.substring(1).toInt();
                    freq_to_send = get_freq((my_stationary_idx - 1) * 2);
                    client->print("Became Stationary " + String(my_stationary_idx) + 
                        ", Changed my frequency to " + String(freq_to_send) + "Hz\n");

                } else if (token.startsWith("N")) {    // change window size
                    config->fourier_window_size = (uint16_t)token.substring(1).toInt();
                    // config->micros_send_duration = ADC_PERIOD * config->fourier_window_size;
                    client->print("Changed window size to " + String(config->fourier_window_size) + "\n");

                } else if (token.startsWith("sd")) {
                    config->micros_send_duration = (uint16_t)token.substring(2).toInt();
                    client->print("Micros send duration changed to " + String(config->micros_send_duration) + "us\n");
                    client->print("in ms: " + String(config->micros_send_duration/1000) + "\n");

                } else if (token.startsWith("t")) {
                    config->dft_threshold = (uint32_t)token.substring(1).toInt();
                    client->print("Changed DFT threshold to " + String(config->dft_threshold) + "\n");

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
                                    
                } else if (token.startsWith("d")) { // change duration to find peak
                    config->duration_to_find_peak = (uint16_t)token.substring(1).toInt();
                    client->print("Changed duration to find peak to " + String(config->duration_to_find_peak) + "us\n");

                } else if (token.startsWith("m")) {  // change max amplitude
                    config->max_amplitude_factor = atof(token.substring(1).c_str());
                    client->print("Changed max amplitude factor to " + String(config->max_amplitude_factor, 3) + "\n");

                }
            }
        }

        if (listen_for_call_and_respond){
            if (is_currently_receiving){
                listener_output_t listener_data = listener->hb();
                if (listener_data.finished){
                    if (listener_data.idx_identified_freq == (my_stationary_idx - 1) * 2){
                        // respond with same frequency received
                        freq_to_send = get_freq(listener_data.idx_identified_freq);
                        client->print("I heard my own frequency (" + String(listener_data.idx_identified_freq) 
                            + ": " + String(freq_to_send) + "Hz) , responding with my own frequency\n");
                        is_currently_receiving = false;
                        ts_start_talking = listener_data.ts_peak + config->period;
                        adc_timer.end();
                        switch_relay_to_send_5ms();
                    }
                    // else if (listener_data.idx_identified_freq == my_stationary_idx * 2 - 1) {
                    //     // respond with frequency received + 1 % 6
                    //     freq_to_send = get_freq((listener_data.idx_identified_freq + 1) % N_FREQUENCIES);
                    //     client->print("I heard (" + String(listener_data.idx_identified_freq) + ": " + 
                    //         String(get_freq(listener_data.idx_identified_freq)) +"Hz) I've been told to trigger the next node (" + String((listener_data.idx_identified_freq + 1) % N_FREQUENCIES) 
                    //         + ": " + String(freq_to_send) + "Hz)\n");
                    //     is_currently_receiving = false;
                    //     ts_start_talking = listener_data.ts_peak + config->period;
                    //     listener->end_adc_timer();
                    //     switch_relay_to_send_5ms();
                    // }
                    else{
                        listener->begin(listener_data.ts_peak + config->period - MICROS_TO_LISTEN_BEFORE_END_OF_PERIOD);
                    }
                }
            } else {    // sending
                send_mode_hb();
            }
        }
        
        // if (micros() - t_last_printed > 1000000){
        //     client->print(uint64ToString(*fourier_counter) + " Hz, Last Value: " + 
        //         String(*last_reading) + ", magnitudes: [" + 
        //         String(frequency_magnitudes[0], 0) + ", " + String(frequency_magnitudes[1], 0) + ", " + 
        //         String(frequency_magnitudes[2], 0) + ", " + String(frequency_magnitudes[3], 0) + ", " + 
        //         String(frequency_magnitudes[4], 0) + ", " + String(frequency_magnitudes[5], 0) + "]\n");
        //     t_last_printed = micros();
        //     *fourier_counter = 0;
        // }
        if (micros() - t_last_printed > 1000000){
            client->print(uint64ToString(*fourier_counter) + " Hz, Last Value: " + 
                String(*last_reading) + ", magnitudes: [" + 
                String(largest_magnitudes[0], 0) + ", " + String(largest_magnitudes[1], 0) + ", " + 
                String(largest_magnitudes[2], 0) + ", " + String(largest_magnitudes[3], 0) + ", " + 
                String(largest_magnitudes[4], 0) + ", " + String(largest_magnitudes[5], 0) + "]\n");
            t_last_printed = micros();
            for (uint8_t i=0; i<6; i++){
                largest_magnitudes[i] = 0;
            }
            *fourier_counter = 0;
        }
    }
    return false; // stay as stationary n
}
