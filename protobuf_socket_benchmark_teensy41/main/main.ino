#include <SPI.h>
#include "src/NativeEthernet/src/NativeEthernet.h"
#include "src/nanopb/pb_common.h"
#include "src/nanopb/pb_encode.h"
#include "src/nanopb/pb_decode.h"
#include "tmp_data.pb.h"

// teensy MAC address. Initialize to zero, as it gets set in teensyMAC automatically.
byte mac[] = {0x0, 0x00, 0x00, 0x00, 0x00, 0x00};

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
//char server[] = "www.google.com";    // name address for Google (using DNS)

IPAddress serverIp(192, 168, 1, 70); //IP address target

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

EthernetClient client;

uint8_t buffer[600];

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
    for (int i=0; i<16; i++){
      pb_encode_varint(&stream, tmpData->data[i]);
    }
}

void setup() {
  Serial.begin(9600);
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
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.print("connecting to ");
  Serial.print(serverIp);
  Serial.println("...");
  
}

void loop(){
  // if you get a connection, report back via serial:
  // if (client.connect(server, 80)) {
  if (client.connect(serverIp, 6969)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }

  #define BENCHMARK_BYTES 1000000
 
  uint64_t sentBytes = 0;
  TmpData tmpData = TmpData_init_zero;
  unsigned long startTime = millis();

  uint64_t i = 0;
  uint64_t total_micros = 0;
  while (sentBytes < BENCHMARK_BYTES){//BENCHMARK_BYTES) { // send 1MB of data
    //Serial.print("uh");
    for (int i = 0; i < 64; i++) {
      tmpData.data[i] = 5555569; // test value, will be real data later
    }
    pb_ostream_t stream;
    //stream = pb_ostream_from_buffer(stream, sizeof(buffer));
    stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    serializeData2(stream, &tmpData);
    //serializeData(stream, &tmpData);
    //Serial.print("a");
    unsigned long beforeSendT = micros();
    int n = client.write(buffer, stream.bytes_written); // takes 0-2 ms
    unsigned long newTimestamp = micros();
    total_micros += newTimestamp - beforeSendT;

    //Serial.println(n);
    sentBytes += n;
    //Serial.println(sentBytes);
    //delay(0);
  }
  unsigned long endTime = millis();

  unsigned long elapsedTime = endTime - startTime;
  Serial.print("fraction spent writing: ");
  Serial.println(total_micros/1000.0/elapsedTime);

  float rate = (float)sentBytes / elapsedTime * 1000.0 / 1000.0; // calculate the upload rate in KB/s
  Serial.print("Sent ");
  Serial.print(sentBytes);
  Serial.print(" bytes in ");
  Serial.print(elapsedTime);
  Serial.print(" ms, rate = ");
  Serial.print(rate);
  Serial.println(" KB/s");

  // close the connection
  client.stop();
  delay(500);
}