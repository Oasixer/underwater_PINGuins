#include "relay.h"
#include <Arduino.h>

void setup_relay(){
    pinMode(RELAY_PIN, OUTPUT);
}

void switch_relay_to_receive_6ms(){
    dac_set_analog_float(-1.0);
    delay(1);
    digitalWrite(RELAY_PIN, RELAY_RECEIVE_MODE);
    delay(5);
}

void switch_relay_to_send_5ms(){
    digitalWrite(RELAY_PIN, RELAY_SEND_MODE);
    delay(5);
}
