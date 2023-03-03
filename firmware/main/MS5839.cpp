#include "MS5839.h"

void MS5839_init(void){

    Wire1.begin();

    //Send reset command
    Wire1.beginTransmission(I2C_ADDRESS);
    Wire1.write(uint8_t(RESET_COMMAND));
    Wire1.endTransmission();
    delay(10); // wait for reset to finish (stolen from https://github.com/bluerobotics/BlueRobotics_MS5837_Library/blob/master/src/MS5837.cpp)

    //Read PROM
    for (uint8_t i = 0; i < 7; i++){
      Wire1.beginTransmission(I2C_ADDRESS);
      Wire1.write(uint8_t(PROM_READ_COMMAND) | (i << 1));
      Wire1.endTransmission();

      MS5839_struct.C[i] = 0;
      Wire1.requestFrom(I2C_ADDRESS, 2);
      while(Wire1.available()){
          MS5839_struct.C[i] <<= 8; //shift last byte over
          MS5839_struct.C[i] |= uint16_t(Wire1.read());
      }
    }
  for (int i = 0; i < 7; i++){
      Serial.println(MS5839_struct.C[i]);
  }    
    

}

void MS5839_start_conversion(measure_t type, resolution_t resolution){
    uint8_t conversion_cmd = 0x40;
    // Set conversion type
    conversion_cmd |= (uint8_t)type << 4;
    // Set conversion resolution
    conversion_cmd |= (uint8_t)resolution <<1;
    // Send it!
    Wire1.beginTransmission(I2C_ADDRESS);
    Wire1.write(conversion_cmd);
    Wire1.endTransmission();
}

// TODO: Check when we last started a conversion! things break if you read before conversion finishes
int32_t MS5839_read_conversion(void){
    //request result
    Wire1.beginTransmission(I2C_ADDRESS);
    Wire1.write(uint8_t(READ_COMMAND));
    Wire1.endTransmission();
    //read result
    int32_t value = 0;
    Wire1.requestFrom(I2C_ADDRESS, 3);
    while(Wire1.available()){
        value <<= 8; //shift last byte over
        value |= Wire1.read();

    }

    return value;
} 

//WARNING! SLOW! will take ~20ms to run!
int32_t MS5839_read_temp(void){
  MS5839_start_conversion(TEMP, RES_8192);
  delay(20); //wait for conversion to complete
  MS5839_struct.D2 = MS5839_read_conversion();
  //Serial.print(" D2: ");
  //Serial.print(MS5839_struct.D2);
  MS5839_struct.dT = MS5839_struct.D2 - MS5839_struct.C[5]*256;
  //Serial.print(" dT: ");
  //Serial.print(MS5839_struct.dT);
  MS5839_struct.temp = 2000+MS5839_struct.dT*MS5839_struct.C[6]/8388608;
  
  return MS5839_struct.temp;

}

//WARNING! SLOW! will take ~20ms to run! USES LAST MEASURED TEMP
int32_t MS5839_read_press(void){
  // get value
  MS5839_start_conversion(PRESS, RES_8192);
  delay(20); //wait for conversion to complete
  MS5839_struct.D1 = MS5839_read_conversion();
  //Serial.print(" D1: ");
  //Serial.print(MS5839_struct.D2);
  //do math
  int64_t offset = ((int64_t)MS5839_struct.C[2]*131072) + ((int64_t)MS5839_struct.dT*MS5839_struct.C[4]/64);
  //Serial.print(" offset: ");
  //Serial.print(offset);
  
  int64_t sensitivity = ((int64_t)MS5839_struct.C[1]*65536) + ((int64_t)MS5839_struct.dT*MS5839_struct.C[3]/128);
  //Serial.print(" Sens: ");
  //Serial.print(sensitivity);
  MS5839_struct.press = (MS5839_struct.D1*sensitivity/2097152 - offset)/32768;
  return MS5839_struct.press;
}

//uses previous press measurement! in mm
int32_t MS5839_calc_depth(void){
  return int32_t((MS5839_struct.press - PRESS_ZERO)*1000/(9.81*WATER_DENSITY)); //N/m2/(N/kg*kg/m3)
}

// returns depth in mm
int32_t get_depth_mm(void){
  MS5839_read_temp();
  MS5839_read_press();
  return MS5839_calc_depth();
}