#include "test_dac_driver.h"

void setup() {
    Serial.begin(9600);
    test_dac_driver_setup();
}

void loop() {
  test_dac_driver_loop();
}
