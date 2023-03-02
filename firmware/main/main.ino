// #include "sender_main.h"
// #include "test_dac_driver.h"
// #include "test_adc_stream.h"
#include "stationary_main.h"
//#include "tcp_client.h"
// #include "rov_main.h"
// #include "fourier.h"
#include "rov_main.h"

#define use_both_servers true // ahmad you should change this
TcpClient client = TcpClient(use_both_servers);

StationaryMain stationary_main = StationaryMain();

config_t config = {500, 25000, 18000, 1000, 1000, 400000, true};

void setup() {
    Serial.begin(9600);


    // test_dac_driver_setup();
    // test_adc_stream_setup();
    // sender_main_setup();
    // fourier_initialize(500);


    client.setup();

    Serial.println("Connect from setup");
    //client.poll_reconnect_if_needed();
    stationary_main.setup(&client, &config);
    // stationary_main_setup(client);
    //rov_main_setup(client);
} 

void loop() {
  // test_fourier_speed_main();
    //client.poll_reconnect_if_needed();
    //client.poll_send_msgs(); // only for ADC
    //rov_main_loop(client);
    stationary_main.loop();
    // sender_main_loop();
  // test_dac_driver_loop();
  // connection_ok = test_adc_stream_loop(connection_ok);
  Serial.println("hi");
  delay(100);
}
