#ifndef FYDP_TCP_CLIENT_H
#define FYDP_TCP_CLIENT_H

#include "src/NativeEthernet/src/NativeEthernet.h"
// typedef struct{
// // teensy MAC address. Initialize to zero, as it gets set in teensyMAC automatically.
//     byte mac[];
//     IPAddress server_ip;
//     IPAddress teensy_ip;
//     IPAddress local_gateway_

// } tcp_client_t;
#include <Arduino.h>

// void init_tcp_client()
// = {0x0, 0x00, 0x00, 0x00, 0x00, 0x00};

#define USE_BOTH_SERVERS 1
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
#define LEAK_DETECT_PIN 14
#define UH_OH_LEAK_IF_MATCH_THIS false


class TcpClient {
    private:
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
        // void send_str_msg();

    public:
        TcpClient();
        void setup();
        void poll_reconnect_if_needed();
        // void poll_incoming_cmds();
        void poll_send_msgs();
        void add_adc_single_reading(uint16_t reading);
        void print(String& message);
};

// static uint8_t[SIZE] console_buffer;
void teensyMAC(uint8_t *mac) {
    for(uint8_t by=0; by<2; by++) mac[by]=(HW_OCOTP_MAC1 >> ((1-by)*8)) & 0xFF;
    for(uint8_t by=0; by<4; by++) mac[by+2]=(HW_OCOTP_MAC0 >> ((3-by)*8)) & 0xFF;
    Serial.printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

bool check_bytes(uint16_t n, EthernetClient& client){
    if (n != BYTES_PER_MSG){
        Serial.print(n);
        Serial.println(" bytes sent => failed to send msg => disconnecting");
        client.close();
        delay(8000);
        return false;
    }
    return true;
}

#endif
