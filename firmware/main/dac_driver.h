#ifndef FYDP_DAC_DRIVER_H
#define FYDP_DAC_DRIVER_H

#include <stdint.h>
#include <math.h>
#include <SPI.h>  // include the SPI library:

#include "constants.h"

void dac_setup(int chip_select_pin, int clr_pin);
void dac_set_analog(uint16_t value);
void dac_set_analog_float(float value);

#endif
