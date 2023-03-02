#include "MS5839.h"

void setup() {
  delay(500);
  // put your setup code here, to run once:  
  Serial.println("Hello World");
  MS5839_init();
  delay(50);


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
