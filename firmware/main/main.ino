// #include "sender_main.h"
// #include "test_dac_driver.h"
// #include "test_adc_stream.h"
#include "stationary_main.h"
#include "tcp_client.h"
// #include "fourier.h"
// #include "rov_main.h"

#define use_both_servers false // ahmad you should change this
TcpClient client = TcpClient(use_both_servers);

void setup() {
    Serial.begin(9600);
    // rov_main_setup();
    // test_dac_driver_setup();
    // test_adc_stream_setup();
    // sender_main_setup();
    // fourier_initialize(500);


    client.setup();
    client.poll_reconnect_if_needed();

    stationary_main_setup(client);
} 

void loop() {
  // rov_main_loop();
  // test_fourier_speed_main();
    client.poll_reconnect_if_needed();
    stationary_main_loop(client);
    // sender_main_loop();
  // test_dac_driver_loop();
  // connection_ok = test_adc_stream_loop(connection_ok);
}
