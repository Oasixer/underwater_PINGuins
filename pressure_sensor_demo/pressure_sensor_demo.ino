#include "MS5839.h"

void setup() {
  // put your setup code here, to run once:  
  Serial.println("Hello World");
  
  MS5839_init();
  delay(50);
  for (int i = 0; i < 6; i++){
      Serial.println(MS5839_struct.C[i]);
  }


}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Temp: ");
  Serial.print(MS5839_read_temp());
  Serial.print("   Pressure: ");
  Serial.print(MS5839_read_press());
  Serial.print("   Depth: ");
  Serial.println(MS5839_calc_depth());
  delay(50);

  


}
