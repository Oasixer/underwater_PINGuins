#include "dac-driver.h"

void setup(int pin){
    dac_pin = pin;
    pinMode(dac_pin, OUTPUT);
    SPI.begin();
}

void set_analog(uint16_t value){
    // Sets the dac output to value. value should be 14 bits (the 2 most significant bits are ignored)
    digitalWrite(dac_pin, LOW);  // take the \bar{CS} low
    SPI.transfer((uint8_t)(value>>6));  // transfer the most significant 8 bits
    SPI.transfer((uint8_t)(value<<2));  // transfer the least significant 6 bits and 2 bits set to zero
    digitalWrite(dac_pin, HIGH);
}

void set_analog_float(float value){
    // Sets the dac output to value. value should be between -1 and 1 (inclusive)
    // if value not between -1 and 1, incorrect output will be set
    uint16_t integer_value = (uint16_t)roundf((value+1.0)*0x1FFF);
    set_analog(integer_value);
}
