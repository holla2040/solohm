#include "solohm.h"
#include <Wire.h> 
#include <WiFi.h>
// library hack, http://stackoverflow.com/questions/6504211/is-it-possible-to-include-a-library-from-another-library-using-the-arduino-ide


SolOhm solohm;

void setup() { 
  solohm.setup("ssid","password");
}

void loop() {
  float d,v;
  char buffer[50];
  d = solohm.dmmRead(DAYSENSOR);
  v = solohm.dmmRead(VBATT3);
  sprintf(buffer,"%d %4d %0.2f\n",millis(),(int)d,v);
  Serial.print(buffer);
  solohm.udpBroadcast(buffer);
  delay(1000);
}



