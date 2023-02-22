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
void teensyMAC(uint8_t *mac) {
    for(uint8_t by=0; by<2; by++) mac[by]=(HW_OCOTP_MAC1 >> ((1-by)*8)) & 0xFF;
    for(uint8_t by=0; by<4; by++) mac[by+2]=(HW_OCOTP_MAC0 >> ((3-by)*8)) & 0xFF;
    Serial.printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

