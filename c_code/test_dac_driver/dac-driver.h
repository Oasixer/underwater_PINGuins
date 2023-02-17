#ifndef FYDP_DAC_DRIVER_H
#define FYDP_DAC_DRIVER_H

#include <stdint.h>
#include <math.h>
#include <SPI.h>  // include the SPI library:

int dac_pin = 0;  //  \bar{CS} pin. used to frame the serial data
int dac_clr_pin = 0;  // \bar{clr} pin. when set to low, sets dac to logic 0.

void dac_setup(int chip_select_pin, int clr_pin);
void dac_set_analog(uint16_t value);
void dac_set_analog_float(float value);

#endif
