#include "dac_driver.h"

int dac_pin = 0;  //  \bar{CS} pin. used to frame the serial data
int dac_clr_pin = 0;  // \bar{clr} pin. when set to low, sets dac to logic 0.
int hv_enable_pin = 0;  // pin to enable high voltage 

void dac_setup(int chip_select_pin, int clr_pin, int hv_enable){
    // store the pin numbers
    dac_pin = chip_select_pin;
    dac_clr_pin = clr_pin;
    hv_enable_pin = hv_enable;

    // configure the pins
    pinMode(dac_pin, OUTPUT);
    pinMode(dac_clr_pin, OUTPUT);
    digitalWrite(dac_clr_pin, HIGH);  // dac_clr_pin should be high unless if you want to reset to 0

    // configure SPI
    delay(10);
    SPI.begin();
    SPI.beginTransaction(SPISettings(25000000, MSBFIRST, SPI_MODE0));
    delay(10);
    
    // safely set high voltage enable pin to high
    dac_set_analog(DAC_CENTER - DAC_AMPLITUDE);
    // delay(1);  // delay to ensure dac set to correct value
    // pinMode(hv_enable_pin, OUTPUT);
    // // digitalWrite(hv_enable_pin, HIGH);
    // delay(3);  // delay to high voltage enabled
}

void dac_set_analog(uint16_t value){
    // Sets the dac output to value. value should be 14 bits (the 2 most significant bits are ignored)
    Serial.println(value);
    digitalWrite(dac_pin, LOW);  // take the \bar{CS} low
    SPI.transfer((uint8_t)(value>>6));  // transfer the most significant 8 bits
    SPI.transfer((uint8_t)(value<<2));  // transfer the least significant 6 bits and 2 bits set to zero
    digitalWrite(dac_pin, HIGH);
}

void dac_set_analog_float(float value){
    // Sets the dac output to value. value should be between -1 and 1 (inclusive)
    // if value not between -1 and 1, incorrect output will be set
    uint16_t integer_value = (uint16_t)roundf(value * DAC_AMPLITUDE + DAC_CENTER);
    dac_set_analog(integer_value);
}
