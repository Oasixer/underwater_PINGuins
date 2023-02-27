// #include "sender_main.h"
// #include "test_dac_driver.h"
// #include "test_adc_stream.h"
#include "stationary_main.h"
// #include "fourier.h"
// #include "rov_main.h"

void setup() {
    Serial.begin(9600);
    // rov_main_setup();
    // test_dac_driver_setup();
    // test_adc_stream_setup();
    // sender_main_setup();
    stationary_main_setup();
    // fourier_initialize(500);
} 

bool connection_ok = false;

void loop() {
  // rov_main_loop();
  // test_fourier_speed_main();
    stationary_main_loop();
    // sender_main_loop();
  // test_dac_driver_loop();
  // connection_ok = test_adc_stream_loop(connection_ok);
}
