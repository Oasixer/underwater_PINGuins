#ifndef FYDP_TEST_DAC_DRIVER_H
#define FYDP_TEST_DAC_DRIVER_H

#include "dac_driver.h"
#include "constants.h"

#include <math.h>

#define TEST_LED 0

#define DAC_PIN 40
#define DAC_CLR 39

#define RELAY_ENABLE 18

#define FREQ 10000

void test_dac_driver_setup();
void test_dac_driver_loop();

#endif