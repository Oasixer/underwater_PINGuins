#include "tcp_client.h"
#include "src/NativeEthernet/src/NativeEthernet.h"

TcpClient::TcpClient(){
    server_ip_try = &server_ip_k;
}

void TcpClient::setup(){
    teensyMAC(mac);

    Ethernet.setStackHeap(1024 * 128);
    Ethernet.setSocketSize(1576 * 8);

    // start the Ethernet connection:
    Serial.println("Initialize Ethernet with DHCP. [If this is the last output, teensy's ethernet prob unplugged]");
    if (Ethernet.begin(mac) == 0) {
        while (true) {
            Serial.println("Failed to configure Ethernet using DHCP");
            delay(100); // do nothing, no point running without Ethernet hardware
        }
    } else {
        Serial.print("    DHCP assigned IP ");
        Serial.println(Ethernet.localIP());
    }

    for (auto i=0; i<BUF_LEN; i++){
        adc_buf[i] = 0;
        // str_buf[i] = 0;
    }

    for (auto i=0; i<BYTES_PER_MSG; i++){
        string_msg[i] = 0;
    }
}

void TcpClient::poll_reconnect_if_needed(){
    if (!client.connected()){
        if (++consecutive_connection_failures > 5){
            #ifdef RESET_ON_FAIL_TO_RECONNECT
                Serial.println("Failed to connect to server too many times, resetting");
                // test_adc_stream_setup();
                delay(1000);
                SCB_AIRCR = 0x05FA0004;
            #endif
        }
        #ifdef USE_BOTH_SERVERS
            if (server_ip_try == &server_ip_a){
                server_ip_try = &server_ip_k;
            } else {
                server_ip_try = &server_ip_a;
            }
        #endif
        #ifndef USE_BOTH_SERVERS
            server_ip_try = &server_ip_k;
        #endif

        if (Ethernet.linkStatus() == LinkOFF) {
            while(true){
                Serial.println("ETHERNET FUCKY WUCKY MAYBE UNPLUGGED");
                delay(500);
            }
        }

        Serial.print("connecting to ");
        Serial.print(*server_ip_try);
        Serial.println("...");
        if (client.connect(*server_ip_try, SERVER_PORT)) {
            Serial.print("Connected to ");
            Serial.println(client.remoteIP());
            connection_timestamp = millis();
        } else {
            Serial.println("Server connection failed");
            delay(200);
            return;
        }
    }
    else{
        consecutive_connection_failures = 0;
    }
}

void TcpClient::send_adc_msg(){
    // send the message
    const uint32_t byte_offset = (cur_adc_msg_being_sent % MSGS_IN_BUF) * BYTES_PER_MSG;
    
    const uint16_t n = client.write(&adc_buf[byte_offset], BYTES_PER_MSG);
    if (!check_bytes(n, client)){
        return;
    }

    #ifdef PRINT_MSG_SEND
    Serial.print("\n>");
    #endif
    cur_adc_msg_being_sent++;
}

void TcpClient::poll_send_msgs(){
    if (cur_adc_msg_being_created - cur_adc_msg_being_sent > 1){
        if (cur_adc_msg_being_created - cur_adc_msg_being_sent > MSGS_IN_BUF){
            #ifdef PRINT_MSG_OVERRUN
                Serial.println("Buffer overrun, skip data & sending msg of ");
                Serial.print("cur_adc_msg_being_created: "); Serial.println(cur_adc_msg_being_created);
                Serial.print("cur_adc_msg_being_sent: "); Serial.println(cur_adc_msg_being_sent);
                Serial.print("created - sent (before): "); Serial.println(cur_adc_msg_being_created - cur_adc_msg_being_sent);
                // cur_adc_msg_being_sent = (cur_adc_msg_being_created - MSGS_IN_BUF + 2);
                // Serial.print("created - sent (after): "); Serial.println(cur_adc_msg_being_created - cur_adc_msg_being_sent);
                // Serial.print("expect delta:"); Serial.println(MSGS_IN_BUF - 2);
                // Serial.print("client.connected(): "); Serial.println(client.connected());
            #endif
            cur_adc_msg_being_sent = (cur_adc_msg_being_created - MSGS_IN_BUF + 2);
            connection_timestamp = millis();
            // measureSpeedFromMsg = cur_adc_msg_being_sent - 1;
            #ifdef DISCONNECT_MSG_OVERRUN
                Serial.println("Buffer overrun, disconnecting.");
                // reset adc_buf to all 0, reset cur_adc_msg_being_created, reset cur_adc_msg_being_sent
                delay(5000);
                return false;
            #endif DISCONNECT_MSG_OVERRUN

            // skip data to catch up by incrementing cur_adc_msg_being_sent
            // 0b0101_1111
            const uint8_t overrun_flag[BYTES_PER_MSG] = {0x5F};
            const uint16_t n = client.write(overrun_flag, BYTES_PER_MSG);
            if (!check_bytes(n, client)){
                // return;
            }
            // +2 from end of ringbuffer to help you catch up to the buffer, dumbass teensy

            // cur_adc_msg_being_sent = (cur_adc_msg_being_created - MSGS_IN_BUF + 2);
            return;
        }
        send_adc_msg();
    }
}

void TcpClient::add_adc_single_reading(uint16_t reading){
    const uint8_t upper_byte = (reading >> 8) & 0xFF;
    const uint8_t lower_byte = reading & 0xFF;
    const uint32_t byte_offset = (cur_adc_msg_being_created % MSGS_IN_BUF) * BYTES_PER_MSG + cur_reading_in_msg * BYTES_PER_READING;

    adc_buf[byte_offset] = upper_byte;
    adc_buf[byte_offset+1] = lower_byte;

    if (++cur_reading_in_msg > READINGS_PER_MSG){
        cur_reading_in_msg = 0;
        cur_adc_msg_being_created++;
    }
}

void TcpClient::print(String& message){
    string_msg[0] = 0b10010000;
    for (auto i=1; i<BYTES_PER_MSG; i++){
        if (i < message.length()+1){
            string_msg[i] = message.charAt(i-1);
        }
        else if (i < last_str_len + 1){
            string_msg[i] = 0x00;
        }
        else{
            break;
        }
    }
    last_str_len = message.length();
    // Serial.println(string_msg[0]);
    const uint16_t n = client.write(string_msg, BYTES_PER_MSG);
    // delay(500);
    if (!check_bytes(n, client)){
    }
}