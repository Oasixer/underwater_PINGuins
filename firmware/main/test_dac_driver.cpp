#include "test_dac_driver.h"
#include <Arduino.h>

uint32_t n_times_wrote_dac_driver = 0;
uint64_t dac_driver_start_time = 0;

void test_dac_driver_setup(){
    dac_setup(DAC_PIN, DAC_CLR, HV_ENABLE);

    pinMode(0, OUTPUT);

    Serial.begin(9600);
}

void test_dac_driver_loop(){
    if (n_times_wrote_dac_driver == 0){
        dac_driver_start_time = micros();
    }
   if (micros() - dac_driver_start_time < 1000000){
      float val = sinf(2 * M_PI * FREQ  / 1000000 * (float)(micros() % (1000000 / FREQ)));
      dac_set_analog_float(val);
      if (val < 0){
        digitalWrite(0, HIGH);
      } else{
        digitalWrite(0, LOW);
      }
      ++n_times_wrote_dac_driver;
   } else {
      Serial.printf("I ran at %i Hz\n", n_times_wrote_dac_driver);
      n_times_wrote_dac_driver = 0;
   }
}
