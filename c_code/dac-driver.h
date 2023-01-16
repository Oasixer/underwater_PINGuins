#ifndef FYDP_DAC_DRIVER_H
#define FYDP_DAC_DRIVER_H

#include <stdint.h>
#include <math.h>
#include <SPI.h>  // include the SPI library:

int dac_pin = 0;  //

void setup(int pin);
void set_analog(uint16_t value);
void set_analog_float(float value);

#endif
