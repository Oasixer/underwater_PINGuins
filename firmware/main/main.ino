// #include "sender_main.h"
// #include "test_dac_driver.h"
// #include "test_adc_stream.h"
#include "stationary_main.h"
#include "listener.h"
//#include "tcp_client.h"
// #include "rov_main.h"
// #include "fourier.h"
#include "rov_main.h"

#define use_both_servers true // ahmad you should change this
TcpClient client = TcpClient(use_both_servers);


config_t config = {
    500, // fourier_window_size
    25000,  // dft_threshold
    2,  // my_frequency_idx
    1000,  // duration_to_find_peak
    1000,  // micros_send_duration
    400000,  // response_timeout_duration
    200000,  // period
    1351.89889440064462178270, // speed_of_sound
    200056.07745910956873558462, // marco_polo_time_delay
    true, // use_rising_edge
    false, // integrate_freq_domain
};

Listener listener = Listener(&config);
StationaryMain stationary_main = StationaryMain(&config, &listener);
void setup() {
    Serial.begin(9600);


    // test_dac_driver_setup();
    // test_adc_stream_setup();
    // sender_main_setup();
    // fourier_initialize(500);


    client.setup();

    Serial.println("Connect from setup");
    //client.poll_reconnect_if_needed();
    stationary_main.setup(&client);
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
