#include "stationary_main.h"
#include "configurations.h"
#include "adc_isr.h"
#include "constants.h"
#include "dac_driver.h"
#include "relay.h"
#include "fourier.h"

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

TcpClient client;

void stationary_main_setup(){
    pinMode(NO_LEAK_PIN, INPUT);
    dac_setup(DAC_PIN, DAC_CLR_PIN, HV_ENABLE_PIN);
    
    setup_relay();
    switch_relay_to_receive();

   adc_timer.begin(adc_timer_callback, ADC_PERIOD);

    adc_setup();
    fourier_initialize(config.fourier_window_size);

    Serial.begin(9600);

    Serial.printf("Starting\n");
    client = TcpClient();
    client.setup();
}

void detect_frequencies() {
    for (uint8_t i = 0; i < N_FREQUENCIES; ++i) {
        if (frequency_magnitudes[i] > config.dft_threshold) {
            // initialize peak finding variables and switch to peak finding state
            curr_max_magnitude = frequency_magnitudes[i];
            ts_peak_finding_timeout = micros() + config.micros_to_find_peak;
            idx_freq_detected = i;
            is_peak_finding = true;

            Serial.print("After ");
            Serial.print(micros() - ts_start_listening);
            Serial.print("us of listening, detected freq ");
            Serial.print(FREQUENCIES[i]);
            Serial.print("Hz, with magnitude ");
            Serial.print(curr_max_magnitude);
            Serial.print(". Will peak find till ");
            Serial.print(ts_peak_finding_timeout);
            Serial.printf(", magnitudes: [%.0f, %.0f, %.0f, %.0f, %.0f, %.0f]\n",
                        frequency_magnitudes[0], frequency_magnitudes[1], frequency_magnitudes[2],
                        frequency_magnitudes[3], frequency_magnitudes[4], frequency_magnitudes[5]);

            return;  // this should only be true for one frequency. there's no point in checking the rest
        }
    }
}

void peak_finding(){
    if (micros() >= ts_peak_finding_timeout){  // finished peak finding
        is_currently_receiving = false; // switch to sending state
        ts_start_talking = ts_peak + INACTIVE_DURATION_BEFORE_TALKING;

        Serial.print("Finished peak finding at ");
        Serial.print(micros());
        Serial.print(". Peak is ");
        Serial.print(ts_peak);
        Serial.print(" with magnitude ");
        Serial.print(curr_max_magnitude);
        Serial.print(", Gonna talk at ");
        Serial.println(ts_start_talking);

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

void receive_mode_hb(){
    if (micros() >= ts_start_listening){  // if not in inactive period
        if (is_peak_finding) {
            peak_finding();
        } else {
            detect_frequencies();
        }
    }
}

void println(String msg){
    
}

void send_data(){
    if (micros() - ts_start_talking < config.micros_send_duration){ // keep sending
        dac_set_analog_float(sinf(2 * M_PI * config.my_frequency  / 1000000 * (float)(micros() % (1000000 / config.my_frequency))));
    } else { // finished beep
        ts_start_listening = micros() + INACTIVE_DURATION_AFTER_BEEP;
        is_currently_receiving = true; // switch to receiving

        Serial.print("sent for ");
        Serial.print(micros() - ts_start_talking);
        Serial.print("us. Finished sending at ");
        Serial.print(micros());
        Serial.print(". Will start listening at ");
        Serial.println(ts_start_listening);
        Serial.println();
        Serial.println();

        switch_relay_to_receive();
        adc_timer.begin(adc_timer_callback, ADC_PERIOD);
    }
}

void send_mode_hb(){
    if (micros() >= ts_start_talking){
        send_data();
    }
}

void stationary_main_loop(){
    if(digitalRead(NO_LEAK_PIN) == THERE_IS_A_LEAK){
        digitalWrite(HV_ENABLE_PIN, LOW);  // turn off high voltage
        switch_relay_to_receive();  // switch to receive mode
        adc_timer.end(); // turn off ADC timer so that we only send Leak Detect messages
        Serial.printf("LEAK DETECTED\n");
    } else {

        String message= "";
        if (client.has_command_available()){
            message = client.read_command_str();
        }
        if (Serial.available() > 0){
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
                    Serial.println("hi");

                } else if (token.startsWith("s")){// stop trying to detect frequencies;
                    listen_for_call_and_respond = false;
                    switch_relay_to_receive();
                    adc_timer.begin(adc_timer_callback, ADC_PERIOD);
                    Serial.println("Stopped");

                } else if (token.startsWith("g")) { // start trying to detect frequencies
                    listen_for_call_and_respond = true;
                    switch_relay_to_receive();
                    adc_timer.begin(adc_timer_callback, ADC_PERIOD);
                    is_peak_finding = false;
                    is_currently_receiving = true;
                    Serial.println("Started");

                } else if (token.startsWith("f")) {  // change my frequency
                    config.my_frequency = (uint16_t)token.substring(1).toInt();
                    Serial.printf("Changed my frequency to %i\n", config.my_frequency);

                } else if (token.startsWith("N")) {  // change window size
                    config.fourier_window_size = (uint16_t)token.substring(1).toInt();
                    config.micros_to_find_peak = ADC_PERIOD * config.fourier_window_size;
                    config.micros_send_duration = ADC_PERIOD * config.fourier_window_size;
                    Serial.printf("Changed window size to %i\n", config.fourier_window_size);

                } else if (token.startsWith("t")) {
                    config.dft_threshold = (uint32_t)token.substring(1).toInt();
                    Serial.printf("Changed DFT threshold to %i\n", config.dft_threshold);

                }
            }
        }

        if (listen_for_call_and_respond){
            if (is_currently_receiving){
                receive_mode_hb();
            } else {  // sending
                send_mode_hb();
            }
        }
        
        if (micros() - t_last_printed > 1000000){
            Serial.printf("%i Hz, Last Value: %i, magnitudes: [%.0f, %.0f, %.0f, %.0f, %.0f, %.0f]\n",
            (uint32_t)fourier_counter, (uint32_t)last_reading,
            frequency_magnitudes[0], frequency_magnitudes[1], frequency_magnitudes[2],
            frequency_magnitudes[3], frequency_magnitudes[4], frequency_magnitudes[5]);
            t_last_printed = micros();
            fourier_counter = 0;
        }
    }
}
