#ifndef FYDP_TCP_CLIENT_H
#define FYDP_TCP_CLIENT_H

#include "src/NativeEthernet/src/NativeEthernet.h"
#include <Arduino.h>

// #define USE_BOTH_SERVERS 1
#define RESET_ON_FAIL_TO_RECONNECT 1

#define INCOMING_MSG_SIZE_BYTES 200
#define BYTES_PER_READING 2
#define READINGS_PER_MSG 400
#define BYTES_PER_MSG (BYTES_PER_READING * READINGS_PER_MSG)
#define MSGS_IN_BUF 100
#define BUF_LEN (BYTES_PER_MSG * MSGS_IN_BUF)
#define MEASURE_SPEED_EVERY_N_MILLIS 2000
#define MSG_PERIOD_MILLIS (readPeriodMicros * READINGS_PER_MSG / 1000.0)
#define MEASURE_SPEED_EVERY_N_MSGS int(MEASURE_SPEED_EVERY_N_MILLIS / MSG_PERIOD_MILLIS)

#define SERVER_PORT 6969


class TcpClient {
    private:
        bool use_ethernet;
        bool use_both_servers;
        IPAddress server_ip_k = IPAddress(192, 168, 1, 70);
        IPAddress server_ip_a = IPAddress(192, 168, 1, 123); //IP address target
        IPAddress* server_ip_try;
        IPAddress myDns = IPAddress(192, 168, 0, 1);
        byte mac[6] = {0x00};
        EthernetClient client;
        uint8_t adc_buf[BUF_LEN];
        // uint8_t str_buf[BUF_LEN];
        uint8_t string_msg[BYTES_PER_MSG];
        uint16_t last_str_len = 0; // for how many bytes to zero out between msgs given they share a buffer
        volatile uint64_t cur_adc_msg_being_created = 0;
        volatile uint64_t cur_adc_msg_being_sent = 0;
        volatile uint16_t cur_reading_in_msg = 0;
        volatile uint64_t cur_str_msg_being_created = 0;
        volatile uint64_t cur_str_msg_being_sent = 0;
        uint64_t connection_timestamp = 0;
        uint8_t consecutive_connection_failures = 0;

        void send_adc_msg();
        void teensyMAC(uint8_t *mac);

    public:
        TcpClient(bool use_both_servers, bool use_ethernet);
        void setup();
        void poll_reconnect_if_needed();
        bool has_cmd_available();
        String get_incoming_cmd();
        void poll_send_msgs();
        void add_adc_single_reading(uint16_t reading);
        void print(String message);
        void send_leak_detected_panic_message();
        bool check_bytes(uint16_t n);
};

// static uint8_t[SIZE] console_buffer;


#endif
