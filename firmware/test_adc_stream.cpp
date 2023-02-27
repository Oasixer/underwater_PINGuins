#include <ADC.h>
#include <IntervalTimer.h>
#include "src/NativeEthernet/src/NativeEthernet.h"
#include "tcp_client.h"
#include "test_adc_stream.h"

// #define PRINT_MSG_SEND 1              // uncomment to print "\n>" after msg send
// #define PRINT_MSG_WAIT 1              // uncomment to print "." when waiting for data to send
#define PRINT_MSG_OVERRUN 1           // uncomment to print info when msg overrun detected
// #define DISCONNECT_MSG_OVERRUN 1   // uncomment to disconnect on msg overrun

IPAddress server_ip_k(192, 168, 1, 70); //IP address target
IPAddress server_ip_a(192, 168, 1, 123); //IP address target
IPAddress* server_ip_try = &server_ip_k;
#define SERVER_PORT 6969

IPAddress ip(192, 168, 0, 177); // Set the static IP address to use if the DHCP fails to assign
IPAddress myDns(192, 168, 0, 1);
// teensy MAC address. Initialize to zero, as it gets set in teensyMAC automatically.
byte mac[] = {0x0, 0x00, 0x00, 0x00, 0x00, 0x00};
EthernetClient client;

const int readPeriodMicros = 2; // us

#define INCOMING_MSG_SIZE_BYTES 200
#define BYTES_PER_READING 2
#define READINGS_PER_MSG 400
#define BYTES_PER_MSG (BYTES_PER_READING * READINGS_PER_MSG)
#define MSGS_IN_BUF 100
#define BUF_LEN (BYTES_PER_MSG * MSGS_IN_BUF)
#define MEASURE_SPEED_EVERY_N_MILLIS 2000
#define MSG_PERIOD_MILLIS (readPeriodMicros * READINGS_PER_MSG / 1000.0)
#define MEASURE_SPEED_EVERY_N_MSGS int(MEASURE_SPEED_EVERY_N_MILLIS / MSG_PERIOD_MILLIS)

uint8_t adc_buf[BUF_LEN];
volatile uint64_t cur_adc_msg_being_created = 0;
volatile uint64_t cur_adc_msg_being_sent = 0;
volatile uint16_t cur_reading_in_msg = 0;
uint64_t measureSpeedFromMsg = 0;

#define LEAK_DETECT_PIN 14
#define UH_OH_LEAK_IF_MATCH_THIS false

const int readPin = A17;
ADC *adc = new ADC();
IntervalTimer timer; // for ADC read ISR @ intervals
int startTimerValue = 0;

int fail_times = 0;

uint64_t connection_timestamp = 0;

void test_adc_stream_setup(){
    pinMode(readPin, INPUT_DISABLE);
    delay(1000);
    teensyMAC(mac);

    Ethernet.setStackHeap(1024 * 128);
    Ethernet.setSocketSize(1576 * 8);


    // start the Ethernet connection:
    Serial.println("Initialize Ethernet with DHCP. [If this is the last output, teensy's ethernet prob unplugged]");
    if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println("Ethernet shield was not found.    Sorry, can't run without hardware. :(");
        while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
        }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
        Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
    } else {
        Serial.print("    DHCP assigned IP ");
        Serial.println(Ethernet.localIP());
    }

    for (auto i=0; i<BUF_LEN; i++){
        adc_buf[i] = 0;
    }

    ///// ADC0 ////
    adc->adc0->setAveraging(0);    // set number of averages
    adc->adc0->setResolution(12); // set bits of resolution
    adc->adc0->setConversionSpeed(
        ADC_CONVERSION_SPEED::HIGH_SPEED); // change the conversion speed
    adc->adc0->setSamplingSpeed(
        ADC_SAMPLING_SPEED::HIGH_SPEED); // change the sampling speed

    Serial.println("Starting Timer");

    // start the timers, if it's not possible, startTimerValuex will be false
    startTimerValue = timer.begin(timer_callback, readPeriodMicros);

    adc->adc0->enableInterrupts(adc_isr);

    Serial.println("Timers started");
}

void timer_callback(void) {
    adc->adc0->startSingleRead(readPin);
}

// when the measurement finishes, this will be called
void adc_isr() {
    const uint16_t reading = adc->adc0->readSingle();
    // adc->adc0->readSingle();
    // const uint16_t reading = 1;

    // split the uint16_t reading into two uint8_t:
    //    - upper byte (most significant, starts w/ four zeros sincer12 bit
    //      reading into 16 bit uint16_t).
    //        -> NOTE: this could be optimized, but for now,
    //           leave the four zeros as a delimiter.
    //   - lower byte (least significant)
    const uint8_t upper_byte = (reading >> 8) & 0xFF;
    const uint8_t lower_byte = reading & 0xFF;
    const uint32_t byte_offset = (cur_adc_msg_being_created % MSGS_IN_BUF) * BYTES_PER_MSG + cur_reading_in_msg * BYTES_PER_READING;

    adc_buf[byte_offset] = upper_byte;
    adc_buf[byte_offset+1] = lower_byte;

    if (++cur_reading_in_msg > READINGS_PER_MSG){
        cur_reading_in_msg = 0;
        cur_adc_msg_being_created++;
    }
    if (adc->adc0->adcWasInUse) {
        // restore ADC config, and restart conversion
        adc->adc0->loadConfig(&adc->adc0->adc_config);
        // avoid a conversion started by this isr to repeat itself
        adc->adc0->adcWasInUse = false;
    }

    #if defined(__IMXRT1062__) // i.MX RT1062 => Teensy 4.0/4.1
    asm("DSB"); // ensure memory access inside the ISR is complete before returning by inserting DSB (Data Sync Barrier) in ASM.
    #endif
}
        
uint8_t string_msg[BYTES_PER_MSG] = {0x00};
uint16_t last_str_len = 0;

bool test_adc_stream_loop(bool connection_active){
    if (!client.connected()){
        if (++fail_times > 5){
            Serial.println("Failed to connect to server too many times, resetting");
            SCB_AIRCR = 0x05FA0004;

            // test_adc_stream_setup();
            delay(1000);
        }
        // if (server_ip_try == &server_ip_a){
        //     server_ip_try = &server_ip_k;
        // } else {
        //     server_ip_try = &server_ip_a;
        // }
        server_ip_try = &server_ip_k;

        // if (!ethernee)
        if (Ethernet.linkStatus() == LinkOFF) {
            Serial.println("ETHERNET FUCKY WUCKY WITHIN??");
        }

        Serial.print("connecting to ");
        Serial.print(*server_ip_try);
        Serial.println("...");
        if (client.connect(*server_ip_try, SERVER_PORT)) {
            Serial.print("connected to ");
            Serial.println(client.remoteIP());
            measureSpeedFromMsg = cur_adc_msg_being_sent;
            connection_timestamp = millis();
        } else {
            Serial.println("Server connection failed, though ethernet connection probably fine?");
            delay(200);
            Serial.println("Retrying connection...");
            return false;
        }
    }
    else{
        fail_times = 0;
    }
    if (digitalReadFast(LEAK_DETECT_PIN) == UH_OH_LEAK_IF_MATCH_THIS){
        Serial.println("Oh no oh fuck oh no");
        const uint8_t leak_flag[BYTES_PER_MSG] = {0xFF};
        const uint16_t n = client.write(leak_flag, BYTES_PER_MSG);
        if (!check_bytes(n, client)){
            Serial.println("LEAK DETECTED AND WE HAVE NO COMMS, GOD HELP YOU SIR/MAAM");
            return false;
        }
        return true;
    }
    #define TEST_STR 1
    #ifdef TEST_STR

    // client.available() returns the number of bytes available to read
    if (client.available() > 0) {
        uint8_t buffer[INCOMING_MSG_SIZE_BYTES];
        int count = client.read(buffer, INCOMING_MSG_SIZE_BYTES);

        // Find the length of the non-zero bytes in the buffer
        int len = 0;
        for (int j = 0; j < count; j++) {
            if (buffer[j] != 0x00) {
                len++;
            }
            else{
                break;
            }
        }

        // Convert the non-zero bytes to a string
        String message = "";
        for (int j = 0; j < len; j++) {
            message += (char) buffer[j];
        }
        Serial.println("Received: " + message);
    }

    String msgstr = "Hi sexy";

    string_msg[0] = 0b10010000;
    for (auto i=1; i<BYTES_PER_MSG; i++){
        if (i < msgstr.length()+1){
            string_msg[i] = msgstr.charAt(i-1);
        }
        else if (i < last_str_len + 1){
            string_msg[i] = 0x00;
        }
        else{
            break;
        }
    }
    last_str_len = msgstr.length();
    // Serial.println(string_msg[0]);
    const uint16_t n = client.write(string_msg, BYTES_PER_MSG);
    delay(500);
    if (!check_bytes(n, client)){
        Serial.println("failed to send string fuck");
        return false;
    }
    else{
        // if (millis() % 100 == 0){
        //     Serial.println("sent string");
        // }
    }
    return true;
    #endif

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
            measureSpeedFromMsg = cur_adc_msg_being_sent - 1;
            #ifdef DISCONNECT_MSG_OVERRUN
                Serial.println("Buffer overrun, disconnecting.");
                // reset adc_buf to all 0, reset cur_adc_msg_being_created, reset cur_adc_msg_being_sent
                delay(5000);
                return false;
            #endif DISCONNECT_MSG_OVERRUN

            // skip data to catch up by incrementing cur_adc_msg_being_sent
            const uint8_t overrun_flag[BYTES_PER_MSG] = {0x5F};
            const uint16_t n = client.write(overrun_flag, BYTES_PER_MSG);
            if (!check_bytes(n, client)){
                return false;
            }
            // +2 from end of ringbuffer to help you catch up to the buffer, dumbass teensy

            // cur_adc_msg_being_sent = (cur_adc_msg_being_created - MSGS_IN_BUF + 2);
            return true;
        }
        #define DOUBLE_SPD 1
        if (cur_adc_msg_being_sent % MEASURE_SPEED_EVERY_N_MSGS == 0 && cur_adc_msg_being_sent-1 > measureSpeedFromMsg){
            Serial.print("free: ");
            // Serial.println(client.free());
            const uint64_t elapsed = millis() - connection_timestamp;
            const uint16_t sentMsgs = (cur_adc_msg_being_sent - 1 - measureSpeedFromMsg) * DOUBLE_SPD;
            float rate_kilobyte_per_s = sentMsgs * BYTES_PER_MSG / elapsed;
            Serial.print("Sent ");
            Serial.print(sentMsgs);
            Serial.print(" ");
            Serial.print(BYTES_PER_MSG);
            Serial.print("byte msgs in ");
            Serial.print(elapsed/1000.0);
            Serial.print("s = ");
            Serial.print(rate_kilobyte_per_s);
            Serial.println(" kB/s");
            connection_timestamp = millis();
            measureSpeedFromMsg = cur_adc_msg_being_sent - 1;
        }
        // send the message
        const uint32_t byte_offset = (cur_adc_msg_being_sent % MSGS_IN_BUF) * BYTES_PER_MSG;
        
        // flag
        // adc_buf[byte_offset] = 0x00;
        const uint16_t n = client.write(&adc_buf[byte_offset], BYTES_PER_MSG); // takes 0-2 ms
        if (!check_bytes(n, client)){
            return false;
        }
        // const uint16_t n2 = client.write(&adc_buf[byte_offset], BYTES_PER_MSG); // takes 0-2 ms
        // if (!check_bytes(n2, client)){
        //     return false;
        // }

        #ifdef PRINT_MSG_SEND
        Serial.print("\n>");
        #endif
        cur_adc_msg_being_sent++;
    }
    else{
        #ifdef PRINT_MSG_WAIT
        Serial.print(".");
        #endif
        // delayMicroseconds(2);
    }
    return true;
}
