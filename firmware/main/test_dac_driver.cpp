#include "test_dac_driver.h"
#include <Arduino.h>

uint32_t n_times_wrote_dac_driver = 0;
uint64_t dac_driver_start_time = 0;

void test_dac_driver_setup(){
    pinMode(TEST_LED, OUTPUT);
    pinMode(DAC_CLR, OUTPUT);
    dac_setup(DAC_PIN, DAC_CLR);

    dac_set_analog(DAC_CENTER - DAC_AMPLITUDE);
    delay(1);  // delay to ensure dac set to correct value
    pinMode(RELAY_ENABLE, OUTPUT);
    digitalWrite(RELAY_ENABLE, HIGH);
    delay(3);  // delay to ensure relay switched

    Serial.begin(9600);
}

void test_dac_driver_loop(){
    if (n_times_wrote_dac_driver == 0){
        dac_driver_start_time = micros();
    }
   if (micros() - dac_driver_start_time < 1000000){
      dac_set_analog_float(sinf(2 * M_PI * FREQ  / 1000000 * (float)(micros() % (1000000 / FREQ))));
      ++n_times_wrote_dac_driver;
   } else {
      Serial.printf("I ran at %i Hz\n", n_times_wrote_dac_driver);
      n_times_wrote_dac_driver = 0;
   }
}
