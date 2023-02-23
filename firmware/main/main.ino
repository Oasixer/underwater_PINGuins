// #include "sender_main.h"
// #include "test_dac_driver.h"
#include "test_adc_stream.h"

void setup() {
    Serial.begin(9600);
    // test_dac_driver_setup();
    test_adc_stream_setup();
    // sender_main_setup();
} 

bool connection_ok = false;

void loop() {
    // sender_main_loop();
  // test_dac_driver_loop();
  connection_ok = test_adc_stream_loop(connection_ok);
}
