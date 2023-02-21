#include <ADC.h>
#include <IntervalTimer.h>
#include "src/NativeEthernet/src/NativeEthernet.h"
#include "src/nanopb/pb_common.h"
#include "src/nanopb/pb_encode.h"
// #include "src/nanopb/pb_decode.h"
#include "tmp_data.pb.h"

// teensy MAC address. Initialize to zero, as it gets set in teensyMAC automatically.
byte mac[] = {0x0, 0x00, 0x00, 0x00, 0x00, 0x00};

IPAddress serverIp(192, 168, 1, 70); //IP address target

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

EthernetClient client;

#define READINGS_PER_MSG sizeof(TmpData::data) / sizeof(uint32_t)
//const uint16_t READINGS_PER_MSG_ = sizeof(TmpData::data) / sizeof(uint32_t);
#define PROTOBUF_MAX_BYTES READINGS_PER_MSG*2+1
//const uint16_t PROTOBUF_MAX_BYTES = READINGS_PER_MSG*2+1;

#define N_MSG_IN_BUFFER 5

// volatile uint8_t msgRingBuffer[N_MSG_IN_BUFFER][PROTOBUF_MAX_BYTES];
uint8_t serialized_buffer[PROTOBUF_MAX_BYTES];
TmpData msg_buffer[N_MSG_IN_BUFFER];
volatile uint16_t data_buffer[N_MSG_IN_BUFFER*READINGS_PER_MSG];
volatile uint16_t curMsgBeingCreated = 0;
volatile uint16_t curMsgBeingSent = 0;
volatile uint16_t curReadingInMsg = 0;

const int readPin = A17;
//const int period = 2; // us
const int period = 10000;

ADC *adc = new ADC(); // adc object

IntervalTimer timer; // timer

int startTimerValue = 0;

// volatile TmpData tmpData = TmpData_init_zero;

// automatically set mac addr of teensy (client)
void teensyMAC(uint8_t *mac) {
    for(uint8_t by=0; by<2; by++) mac[by]=(HW_OCOTP_MAC1 >> ((1-by)*8)) & 0xFF;
    for(uint8_t by=0; by<4; by++) mac[by+2]=(HW_OCOTP_MAC0 >> ((3-by)*8)) & 0xFF;
    Serial.printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void serializeData(pb_ostream_t& stream, TmpData* tmpData){
  if (!pb_encode(&stream, TmpData_fields, tmpData)){
    Serial.printf("encode fail: %s\n", PB_GET_ERROR(&stream));
    return;
  }
}

void serializeData2(pb_ostream_t& stream, TmpData* tmpData) {
    // Encode the fields directly to the buffer
    pb_encode_tag(&stream, PB_WT_VARINT, TmpData_data_tag);
    for (int i=0; i<READINGS_PER_MSG; i++){
      pb_encode_varint(&stream, tmpData->data[i]);
    }
}

void setup() {
  pinMode(readPin, INPUT_DISABLE);

  Serial.begin(9600);

  delay(1000);

  teensyMAC(mac);

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
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
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }

  for (int i=0; i<N_MSG_IN_BUFFER; i++){
    TmpData tmpData = TmpData_init_zero;
    msg_buffer[i] = tmpData;
  }

  // readings_remaining = READINGS_PER_MSG;

  ///// ADC0 ////
  adc->adc0->setAveraging(0);  // set number of averages
  adc->adc0->setResolution(12); // set bits of resolution
  adc->adc0->setConversionSpeed(
      ADC_CONVERSION_SPEED::HIGH_SPEED); // change the conversion speed
  adc->adc0->setSamplingSpeed(
      ADC_SAMPLING_SPEED::HIGH_SPEED); // change the sampling speed

  Serial.println("Starting Timer");

  // start the timers, if it's not possible, startTimerValuex will be false
  startTimerValue = timer.begin(timer_callback, period);

  adc->adc0->enableInterrupts(adc_isr);

  Serial.println("Timers started");
}

void loop() {
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.print("connecting to ");
  Serial.print(serverIp);
  Serial.println("...");
  if (client.connect(serverIp, 6969)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
  } else {
    Serial.println("connection failed");
  }

  while(true){
  //if (curMsgBeingCreated - curMsgBeingSent > 1){
  //   pb_ostream_t stream;
  //   stream = pb_ostream_from_buffer(serialized_buffer, PROTOBUF_MAX_BYTES);
  //   serializeData2(stream, &msg_buffer[curMsgBeingSent]);
  //   int n = client.write(serialized_buffer, stream.bytes_written); // takes 0-2 ms
  //   curMsgBeingSent += 1;
  // 
   //}
   //else{
    Serial.println("waiting for data");
    delay(1000);
  // }
  }
  // if ()
  // if (micros() - last_micros_printed >= 1000000){
  //   Serial.printf("ISR was called %i times, latest value %i\n", (uint32_t)num_times_isr_called, latest_adc_value);
  //   num_times_isr_called = 0;
  //   last_micros_printed = micros();
  // }
}

// This function will be called with the desired frequency
// start the measurement
void timer_callback(void) {
  adc->adc0->startSingleRead(readPin);
}

// when the measurement finishes, this will be called
void adc_isr() {
  curReadingInMsg = adc->adc0->readSingle();
  //msg_buffer[curMsgBeingCreated].data[curReadingInMsg] = (uint32_t)
  //msg_buffer[curMsgBeingCreated].data[curReadingInMsg] = curReadingInMsg;
  //curReadingInMsg++;// = ++curReadingInMsg;
  //if (curReadingInMsg > READINGS_PER_MSG){
  //  curMsgBeingCreated++; // TODO mutex this
  //  curReadingInMsg = 0;
  //}
  if (adc->adc0->adcWasInUse) {
    // restore ADC config, and restart conversion
    adc->adc0->loadConfig(&adc->adc0->adc_config);
    // avoid a conversion started by this isr to repeat itself
    adc->adc0->adcWasInUse = false;
  }

  // digitalWriteFast(ledPin+2, !digitalReadFast(ledPin+2));

#if defined(__IMXRT1062__) // Teensy 4.0
  asm("DSB");
#endif
}
