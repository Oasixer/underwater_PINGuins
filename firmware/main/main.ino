// #include "sender_main.h"
// #include "test_dac_driver.h"
// #include "test_adc_stream.h"
#include "configurations.h"
#include "stationary_main.h"
#include "rov_main.h"
#include "listener.h"
#include "talker.h"
#include "tcp_client.h"
// #include "fourier.h"

#define USE_ETHERNET true
#define use_both_servers true // ahmad you should change this
TcpClient client = TcpClient(use_both_servers, USE_ETHERNET);

config_t config = {
    500, // fourier_window_size
    25000,  // dft_threshold
    1000,  // duration_to_find_peak
    1000,  // micros_send_duration
    400000,  // response_timeout_duration
    200000,  // period
    1351.89889440064462178270, // speed_of_sound
    200056.07745910956873558462, // marco_polo_time_delay
    true, // use_rising_edge
    false, // integrate_freq_domain
};

Talker talker = Talker(&config);
Listener listener = Listener(&config);
StationaryMain stationary_main = StationaryMain(&config, &listener, &client);
RovMain rov_main = RovMain(&config, &listener, &client, &talker);
// StationaryMain stationary_main = StationaryMain(&config, &listener);
// RovMain rov_main = RovMain(&config, &listener);
// void setup() {
//     Serial.begin(9600);

bool USE_ROV = false;

void setup() {
    Serial.begin(9600);

    client.setup();
    if (USE_ETHERNET){
      client.poll_reconnect_if_needed();
    }

    Serial.println("Connect from setup");
    if (USE_ROV){
        rov_main.setup();
    } else {
        stationary_main.setup();
    }
} 

void loop() {
  // test_fourier_speed_main();
    if (USE_ETHERNET){
      client.poll_reconnect_if_needed();
    }
    //client.poll_send_msgs(); // only for ADC

    bool change_rov_state = false;
    if (USE_ROV){
        change_rov_state = rov_main.loop();
    } else {
        change_rov_state = stationary_main.loop();
    }

    if (change_rov_state){
        if (USE_ROV){
            rov_main.shutdown();
            stationary_main.setup();
            USE_ROV = false;
        }
        else{ // PREVIOUS STATE WAS STATIONARY
            stationary_main.shutdown();
            rov_main.setup();
            USE_ROV = true;
        }
    }
}
