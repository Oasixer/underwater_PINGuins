#include "stationary_main.h"
#include "configurations.h"
#include "adc_isr.h"
#include "constants.h"
#include "dac_driver.h"
#include "relay.h"
#include "fourier.h"
#include "printing.h"

#include "tcp_client.h"
#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <IntervalTimer.h>


// for receiving
IntervalTimer adc_timer; // for ADC read ISR @ intervals

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

extern float frequency_magnitudes[N_FREQUENCIES];

// variables used for checking health
extern uint64_t fourier_counter;
extern uint16_t last_reading;
uint64_t t_last_printed = 0;

// configurations
extern config_t config;

void stationary_main_setup(TcpClient& client){
    pinMode(NO_LEAK_PIN, INPUT);
    dac_setup(DAC_PIN, DAC_CLR_PIN, HV_ENABLE_PIN);
    
    setup_relay();
    client.print("Setup relay\n");
    switch_relay_to_receive();

    adc_timer.begin(adc_timer_callback, ADC_PERIOD);

    adc_setup();
    client.print("Setup adc and timer\n");
    fourier_initialize(config.fourier_window_size);

    client.print("Setup fourier\n");
    // client = TcpClient();
    // client.setup();
    // client.print("Starting\n");
}

void detect_frequencies(TcpClient& client) {
    for (uint8_t i = 0; i < N_FREQUENCIES; ++i) {
        if (frequency_magnitudes[i] > config.dft_threshold) {
            // initialize peak finding variables and switch to peak finding state
            curr_max_magnitude = frequency_magnitudes[i];
            ts_peak = micros();
            ts_peak_finding_timeout = micros() + config.micros_to_find_peak;
            idx_freq_detected = i;
            is_peak_finding = true;

            client.print("After " + uint64ToString(micros() - ts_start_listening) + 
                "us of listening, detected freq " + String(FREQUENCIES[i]) + 
                "Hz, with magnitude " + String(curr_max_magnitude, 0) + ", with magnitudes: [" + 
                String(frequency_magnitudes[0], 0) + ", " + String(frequency_magnitudes[1], 0) + ", " + 
                String(frequency_magnitudes[2], 0) + ", " + String(frequency_magnitudes[3], 0) + ", " + 
                String(frequency_magnitudes[4], 0) + ", " + String(frequency_magnitudes[5], 0) + "]\n");

            return;  // this should only be true for one frequency. there's no point in checking the rest
        }
    }
}

void peak_finding(){
    if (config.use_rising_edge || micros() >= ts_peak_finding_timeout){  // finished peak finding
        is_currently_receiving = false; // switch to sending state
        ts_start_talking = ts_peak + INACTIVE_DURATION_BEFORE_TALKING;

        // client.print("Finished peak finding at " + uint64ToString(micros()) + 
        //     ". Peak is" + uint64ToString(ts_peak) + " with magnitude " + 
        //     String(curr_max_magnitude, 0) + "\n");

        curr_max_magnitude = 0;  // reset to zero for next time
        is_peak_finding = false;  // start next time not in peak finding state
        
        switch_relay_to_send();
    } else {
        if (frequency_magnitudes[idx_freq_detected] > curr_max_magnitude){
            curr_max_magnitude = frequency_magnitudes[idx_freq_detected];
            ts_peak = micros();
        }
    }
}

void receive_mode_hb(TcpClient& client){
    if (micros() >= ts_start_listening){  // if not in inactive period
        if (is_peak_finding) {
            peak_finding();
        } else {
            detect_frequencies(client);
        }
    }
}

void send_data(){
    if (micros() - ts_start_talking < config.micros_send_duration){ // keep sending
        dac_set_analog_float(sinf(2 * M_PI * config.my_frequency  / 1000000 * (float)(micros() % (1000000 / config.my_frequency))));
    } else { // finished beep
        ts_start_listening = micros() + INACTIVE_DURATION_AFTER_BEEP;
        is_currently_receiving = true; // switch to receiving

        // client.print("sent for " + uint64ToString(micros() - ts_start_talking) + 
        //     "us. Finished sending at " + uint64ToString(micros()) + 
        //     ". Will start listening at " + uint64ToString(ts_start_talking) + "\n");

        switch_relay_to_receive();
        adc_timer.begin(adc_timer_callback, ADC_PERIOD);
    }
}

void send_mode_hb(){
    if (micros() >= ts_start_talking){
        send_data();
    }
}

void stationary_main_loop(TcpClient& client){
    if(digitalRead(NO_LEAK_PIN) == THERE_IS_A_LEAK){
        digitalWrite(HV_ENABLE_PIN, LOW);  // turn off high voltage
        switch_relay_to_receive();  // switch to receive mode
        adc_timer.end(); // turn off ADC timer so that we only send Leak Detect messages
        client.send_leak_detected_panic_message();
    } else {

        String message= "";
        if (client.has_cmd_available()){
            message = client.get_incoming_cmd();
        }
        else if (Serial.available() > 0){
            message = Serial.readStringUntil(message_terminator);
        }
        if (message.length() > 0){
            // Read the incoming message from the serial port

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
                if (token.startsWith("h")){  // just a ping
                    client.print("hi\n");
                } else if (token.startsWith("s")){// stop trying to detect frequencies;
                    listen_for_call_and_respond = false;
                    switch_relay_to_receive();
                    adc_timer.begin(adc_timer_callback, ADC_PERIOD);
                    client.print("Stopped\n");
                } else if (token.startsWith("g")) { // start trying to detect frequencies
                    listen_for_call_and_respond = true;
                    switch_relay_to_receive();
                    adc_timer.begin(adc_timer_callback, ADC_PERIOD);
                    is_peak_finding = false;
                    is_currently_receiving = true;
                    client.print("Started\n");

                } else if (token.startsWith("f")) {  // change my frequency
                    config.my_frequency = (uint16_t)token.substring(1).toInt();
                    client.print("Changed my frequency to " + String(config.my_frequency) + "\n");

                } else if (token.startsWith("N")) {  // change window size
                    config.fourier_window_size = (uint16_t)token.substring(1).toInt();
                    config.micros_to_find_peak = ADC_PERIOD * config.fourier_window_size;
                    config.micros_send_duration = ADC_PERIOD * config.fourier_window_size;
                    client.print("Changed window size to " + String(config.fourier_window_size) + "\n");

                } else if (token.startsWith("t")) {
                    config.dft_threshold = (uint32_t)token.substring(1).toInt();
                    client.print("Changed DFT threshold to " + String(config.dft_threshold) + "\n");

                } else if (token.startsWith("r")) { // use rising edge 
                    uint8_t use_rising_edge = (uint8_t)token.substring(1).toInt();
                    config.use_rising_edge = use_rising_edge > 0;
                    client.print("Use rising edge set to " + String(config.use_rising_edge) + "\n");
                }
            }
        }

        if (listen_for_call_and_respond){
            if (is_currently_receiving){
                receive_mode_hb(client);
            } else {  // sending
                send_mode_hb();
            }
        }
        
        if (micros() - t_last_printed > 1000000){
            client.print(uint64ToString(fourier_counter) + " Hz, Last Value: " + 
                String(last_reading) + ", magnitudes: [" + 
                String(frequency_magnitudes[0], 0) + ", " + String(frequency_magnitudes[1], 0) + ", " + 
                String(frequency_magnitudes[2], 0) + ", " + String(frequency_magnitudes[3], 0) + ", " + 
                String(frequency_magnitudes[4], 0) + ", " + String(frequency_magnitudes[5], 0) + "]\n");
            t_last_printed = micros();
            fourier_counter = 0;
        }
    }
}
