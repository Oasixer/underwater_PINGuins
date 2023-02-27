#include "rov_main.h"
#include "configurations.h"
#include "adc_isr.h"
#include "constants.h"
#include "dac_driver.h"
#include "relay.h"
#include "fourier.h"
#include "printing.h"
#include "TcpClient.h"

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>


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

uint64_t ts_stop_continuous_transmission = 0;
uint64_t ts_response_timeout = 0;

uint8_t idx_freq_detected = 0;

float curr_max_magnitude = 0;

extern float frequency_magnitudes[N_FREQUENCIES];

// configurations
extern config_t config;

// buffer to store trip times
#define MAX_N_TRIPS 500
uint64_t trip_times[MAX_N_TRIPS] = {0};
uint16_t n_talks_done = 0;
uint16_t n_talks_command = 0;

void rov_main_setup(TcpClient& client){
    pinMode(NO_LEAK_PIN, INPUT);
    dac_setup(DAC_PIN, DAC_CLR_PIN, HV_ENABLE_PIN);
    
    setup_relay();
    switch_relay_to_send();

    adc_setup();
    fourier_initialize(config.fourier_window_size);

    // Serial.begin(9600);

    config.my_frequency = 25000;
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

// g 100
// 

void reset_send_receive(TcpClient& client){
    uint64_t trip_time = ts_peak - ts_start_talking;
    trip_times[n_talks_done] = trip_time; // store for later
    client.print("Trip Time: " + uint64ToString(trip_time) + "us\n");

    curr_max_magnitude = 0;  // reset to zero for next time
    is_peak_finding = false;  // start next time not in peak finding state
    is_currently_receiving = false; // switch to sending state for next time
    adc_timer.end();
    switch_relay_to_send();

    if (++n_talks_done < n_talks_command){  // still have to send receive more
        ts_start_talking = micros() + INACTIVE_DURATION_BEFORE_TALKING;
        ts_response_timeout = ts_start_talking + config.response_timeout_duration;

    } else {  // did all the commanded send receives
        client.print("Finished send receives " + String(n_talks_done) + " times. Trip times in us are:\n");
        String msg = "[";
        for (uint16_t i = 0; i < n_talks_done; ++i){
            msg += uint64ToString(trip_times[i]); 
            msg += "; ";
            if (i % 10 == 9){
                client.print(msg + "\n");
                msg = "";
            }
        }
        client.print(msg + "]\n\n");
        n_talks_command = 0;
        n_talks_done = 0;
    }
}

void rov_peak_finding(TcpClient& client){
    if (config.use_rising_edge || micros() >= ts_peak_finding_timeout){  // finished peak finding
        reset_send_receive(client);
    } else {
        if (frequency_magnitudes[idx_freq_detected] > curr_max_magnitude){
            curr_max_magnitude = frequency_magnitudes[idx_freq_detected];
            ts_peak = micros();
        }
    }
}

void rov_receive_mode_hb(TcpClient& client){
    if (micros() >= ts_response_timeout){ // assume wont get a response
        ts_peak = -1;  // indicate that was not able to find peak
        client.print("TIMEOUT didn't hear a response\n");
        reset_send_receive(client);
    } else {
        if (micros() >= ts_start_listening){  // if not in inactive period
            if (is_peak_finding) {
                rov_peak_finding(client);
            } else {
                detect_frequencies(client);
            }
        }
    }
}

void rov_send_mode_hb(TcpClient& client){
    if (micros() >= ts_start_talking){ // send data
        if (micros() - ts_start_talking < config.micros_send_duration){ // keep sending
            dac_set_analog_float(sinf(2 * M_PI * config.my_frequency  / 1000000 * (float)(micros() % (1000000 / config.my_frequency))));

        } else { // finished beep
            ts_start_listening = micros() + INACTIVE_DURATION_AFTER_BEEP;
            is_currently_receiving = true; // switch to receiving
            switch_relay_to_receive();
            adc_timer.begin(adc_timer_callback, ADC_PERIOD);
        }
    }
}

void rov_main_loop(TcpClient& client){
    if(digitalRead(NO_LEAK_PIN) == THERE_IS_A_LEAK){
        digitalWrite(HV_ENABLE_PIN, LOW);  // turn off high voltage
        switch_relay_to_receive();  // switch to receive mode
        adc_timer.end(); // turn off ADC timer so that we only send Leak Detect messages
        // client.print("LEAK DETECTED\n");
        client.send_leak_detected_panic_message();
    } else {
        String message= "";
        if (client.has_cmd_available()){
            message = client.get_incoming_cmd();
        }
        else if (Serial.available() > 0){
            message = Serial.readStringUntil(message_terminator);
        }
        if (message.length() > 0) {

            // Read the incoming message from the serial port
            String message = Serial.readStringUntil(message_terminator);

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

                } else if (token.startsWith("s")){ // stop everything and go to default state;
                    is_transmit_continuously = false;
                    switch_relay_to_send();
                    adc_timer.end();
                    n_talks_done = 0;
                    n_talks_command = 0;
                    is_currently_receiving = false;
                    is_peak_finding = false;
                    client.print("Stopped\n");

                } else if (token.startsWith("g")) { // talk for x times
                    n_talks_command = (uint16_t)token.substring(1).toInt();
                    ts_start_talking = micros();  // start talking now
                    client.print("Gonna talk " + String(n_talks_command) + " times\n");
                    ts_response_timeout = ts_start_talking + config.response_timeout_duration;

                } else if (token.startsWith("f")) {  // change my frequency
                    config.my_frequency = (uint16_t)token.substring(1).toInt();
                    client.print("Changed my frequency to " + String(config.my_frequency) + "\n");

                } else if (token.startsWith("N")) {  // change window size
                    config.fourier_window_size = (uint16_t)token.substring(1).toInt();
                    config.micros_to_find_peak = ADC_PERIOD * config.fourier_window_size;
                    config.micros_send_duration = ADC_PERIOD * config.fourier_window_size;
                    client.print("Changed window size to " + String(config.fourier_window_size) + "\n");

                } else if (token.startsWith("t")) {  // change threshold
                    config.dft_threshold = (uint32_t)token.substring(1).toInt();
                    client.print("Changed DFT threshold to " + String(config.dft_threshold) + "\n");

                } else if (token.startsWith("o")) { // set timeout
                    config.response_timeout_duration = (uint32_t)token.substring(1).toInt();
                    client.print("Changed response timeout to " + String(config.response_timeout_duration) + "us after talking starts\n");

                } else if (token.startsWith("c")){ // transmit continuously
                    uint32_t millis_to_continuously_transmit = (uint32_t)token.substring(1).toInt();
                    client.print("Will transmit continuously for " + String(millis_to_continuously_transmit) + "ms\n");
                    is_transmit_continuously = true;
                    ts_stop_continuous_transmission = micros() + millis_to_continuously_transmit*1000;
                
                } else if (token.startsWith("r")) { // use rising edge 
                    uint8_t use_rising_edge = (uint8_t)token.substring(1).toInt();
                    config.use_rising_edge = use_rising_edge > 0;
                    client.print("Use rising edge set to " + String(config.use_rising_edge) + "\n");
                }
            }
        }

        if (is_transmit_continuously) {  // if the command is to transmit continuously
            if (micros() < ts_stop_continuous_transmission) { // should still transmit
                dac_set_analog_float(sinf(2 * M_PI * config.my_frequency  / 1000000 * (float)(micros() % (1000000 / config.my_frequency))));
            } else { // stop transmitting
                client.print("Finished continuous transmission\n");
                is_transmit_continuously = false;
            }

        } else if (n_talks_done < n_talks_command){  // if still have to send receive more
            if (is_currently_receiving){
                rov_receive_mode_hb(client);
            } else {  // sending
                rov_send_mode_hb(client);
            }

        } // else do nothing
    }
}
