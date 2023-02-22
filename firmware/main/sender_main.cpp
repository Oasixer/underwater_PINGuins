#include "sender_main.h"
#include <Arduino.h>

#define DAC_PIN 40
#define DAC_CLR_PIN 39
#define HV_ENABLE_PIN 18
#define RELAY 25
#define NO_LEAK_PIN 14

// Define the message format
const char message_delimiter = ' ';
const char message_terminator = '\n';

// Define variables to read from serial
uint16_t freq = 10000;
uint32_t duration = -1, period = -1, active_duration = -1;

// Define timing variables
uint64_t stop_time = 0, active_start_time = -1, active_end_time = 0;

// Define variable to check for leaks
bool no_leak = true;

void sender_main_setup(){
    pinMode(NO_LEAK_PIN, INPUT);
    if (digitalRead(NO_LEAK_PIN)){
        dac_setup(DAC_PIN, DAC_CLR_PIN, HV_ENABLE_PIN);

        // switch relay to talk mode
        pinMode(RELAY, OUTPUT);
        digitalWrite(RELAY, HIGH);
    }

    Serial.begin(9600);
    Serial.println("Started");
}

void sender_main_loop() {
    no_leak = digitalRead(NO_LEAK_PIN);
    if (!no_leak){  // oh shit, there's a leak
        while (true){
            digitalWrite(HV_ENABLE_PIN, LOW);  // turn off high voltage
            digitalWrite(RELAY, LOW);  // switch to receive mode
            Serial.println("LEAK DETECTED");
        }
    } else {  // no leak
        if (Serial.available() > 0) {
            // Read the incoming message from the serial port
            String message = Serial.readStringUntil(message_terminator);

            if (message.indexOf('h') >= 0){  // just a ping
                Serial.println("hi");
            } else if (message.indexOf('s') >= 0){ // stop driving the piezo immediately;
                stop_time = 0;
                Serial.println("Stopped");
            } else {  // command to drive the piezo. Decode it
                // start by initializing variables to default
                duration = -1;
                period = -1;
                active_duration = -1;

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
                    if (token.startsWith("f")) {
                        freq = (uint16_t)token.substring(1).toInt();
                    } else if (token.startsWith("d")) {
                        duration = (uint32_t)token.substring(1).toInt();
                    } else if (token.startsWith("p")) {
                        period = (uint32_t)token.substring(1).toInt();
                    } else if (token.startsWith("a")) {
                        active_duration = (uint32_t)token.substring(1).toInt();
                    }
                }

                // change timing variables
                if (duration != -1){ // duration specified
                    stop_time = micros() + (uint64_t)duration * 1000; // duration is in millis
                } else { // duration not specified
                    stop_time = -1;  // infinite duration
                }            
                active_start_time = micros();  // start sending immediately
                if (period != -1 && active_duration != -1){ // both period and active duration specified
                    active_end_time = micros() + active_duration;
                } else {
                    active_end_time = stop_time;  // active till end
                }

                Serial.printf("Sending at %iHz for %ims. p: %ius. a: %ius\n", (uint32_t)freq, duration, period, active_duration);
            }
        }

        if (micros() < stop_time){  // havent reached end of duration yet
            if (micros() >= active_start_time){
                if (micros() >= active_end_time) { // increment active_start_time and active_end_time
                    active_start_time += period;
                    active_end_time += period;
                } else {  // active period
                    dac_set_analog_float(sinf(2 * M_PI * freq  / 1000000 * (float)(micros() % (1000000 / freq))));
                }
            }
        }
    }
}

