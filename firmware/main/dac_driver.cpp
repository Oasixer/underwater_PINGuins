#include "dac_driver.h"

int dac_pin = 0;  //  \bar{CS} pin. used to frame the serial data
int dac_clr_pin = 0;  // \bar{clr} pin. when set to low, sets dac to logic 0.

void dac_setup(int chip_select_pin, int clr_pin){
    // store the pin numbers
    dac_pin = chip_select_pin;
    dac_clr_pin = clr_pin;

    // configure the pins
    pinMode(dac_pin, OUTPUT);
    pinMode(dac_clr_pin, OUTPUT);
    digitalWrite(dac_clr_pin, HIGH);  // dac_clr_pin should be high unless if you want to reset to 0

    // configure SPI
    SPI.begin();
    SPI.beginTransaction(SPISettings(25000000, MSBFIRST, SPI_MODE0));
}

void dac_set_analog(uint16_t value){
    // Sets the dac output to value. value should be 14 bits (the 2 most significant bits are ignored)
    digitalWrite(dac_pin, LOW);  // take the \bar{CS} low
    SPI.transfer((uint8_t)(value>>6));  // transfer the most significant 8 bits
    SPI.transfer((uint8_t)(value<<2));  // transfer the least significant 6 bits and 2 bits set to zero
    digitalWrite(dac_pin, HIGH);
}

void dac_set_analog_float(float value){
    // Sets the dac output to value. value should be between -1 and 1 (inclusive)
    // if value not between -1 and 1, incorrect output will be set
    uint16_t integer_value = (uint16_t)roundf((value+1.0)*0x1FFF);  // TODO: check the math
    dac_set_analog(integer_value);
}
