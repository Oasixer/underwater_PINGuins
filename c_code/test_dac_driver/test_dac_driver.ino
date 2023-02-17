#include "dac-driver.h"

#define TEST_LED 0

#define DAC_PIN 40
#define DAC_CLR 39

#define FREQ 20000

#include <math.h>

uint32_t i = 0;
uint64_t start_time = 0;

void setup(){
    pinMode(TEST_LED, OUTPUT);
    pinMode(DAC_CLR, OUTPUT);
    dac_setup(DAC_PIN, DAC_CLR);

    Serial.begin(9600);
}

void loop(){
  // digitalWrite(TEST_LED, HIGH);  // turn the LED on (HIGH is the voltage level)
  // dac_set_analog(0);
  // delay(1000);                      // wait for a second
  // dac_set_analog(0x3FFF);
  // digitalWrite(TEST_LED, LOW);   // turn the LED off by making the voltage LOW
  // delay(1000);                      // wait for a second
    if (i == 0){
        start_time = micros();
    }
   if (micros() - start_time < 1000000){
      dac_set_analog_float(sinf(2 * M_PI * FREQ * ((float)micros() / 1000000)));
      ++i;
   } else {
      Serial.printf("I ran at %i Hz\n", i);
      i = 0;
   }
}
