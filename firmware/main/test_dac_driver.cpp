#include "test_dac_driver.h"
#include <Arduino.h>

#define DAC_PIN 40
#define DAC_CLR 39
#define HV_ENABLE_PIN 18
#define RELAY_PIN 25

#define FREQ 10000

uint32_t n_times_wrote_dac_driver = 0;
uint64_t dac_driver_start_time = 0;

void test_dac_driver_setup(){
    dac_setup(DAC_PIN, DAC_CLR, HV_ENABLE_PIN);
    
    // switch relay to talk mode
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);

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
