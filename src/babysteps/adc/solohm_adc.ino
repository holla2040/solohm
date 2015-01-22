#include "solohm.h"

#define VBATT1SCALE  4.136/3057.0
#define VBATT2SCALE  8.279/3351.0
#define VBATT3SCALE 12.413/3130.0

int amuxRead(uint8_t channel) {
  switch (channel) {
  case DAYSENSORSCALED:      
    digitalWrite(AMUXS0,0);
    digitalWrite(AMUXS1,0);
    digitalWrite(AMUXS2,0);
    break;
  case VBATT1SCALED:      
    digitalWrite(AMUXS0,1);
    digitalWrite(AMUXS1,0);
    digitalWrite(AMUXS2,0);
    break;
  case VBATT2SCALED:      
    digitalWrite(AMUXS0,0);
    digitalWrite(AMUXS1,1);
    digitalWrite(AMUXS2,0);
    break;
  case VBATT3SCALED:      
    digitalWrite(AMUXS0,1);
    digitalWrite(AMUXS1,1);
    digitalWrite(AMUXS2,0);
    break;
  case V3P3SCALED:      
    digitalWrite(AMUXS0,0);
    digitalWrite(AMUXS1,0);
    digitalWrite(AMUXS2,1);
    break;
  case V5SWSCALED:      
    digitalWrite(AMUXS0,1);
    digitalWrite(AMUXS1,0);
    digitalWrite(AMUXS2,1);
    break;
  }
  delay(100);
  return analogRead(AMUXOUT);
}

void setup() {
  Serial.begin(115200);
  pinMode(VPANELSCALED,INPUT);
  pinMode(IPANELSCALED,INPUT);
  pinMode(AMUXOUT,INPUT);
  
  pinMode(AMUXS0,OUTPUT);
  pinMode(AMUXS1,OUTPUT);
  pinMode(AMUXS2,OUTPUT);
  Serial.println("solohm_adc begin");
}

void loop() {
  int v;
  float voltage;
/*
  v = analogRead(VPANELSCALED);
  Serial.print("VPANELSCALED    ");
  Serial.println(v,DEC);

  v = analogRead(IPANELSCALED);
  Serial.print("IPANELSCALED    ");
  Serial.println(v,DEC);

  v = amuxRead(DAYSENSORSCALED);
  Serial.print("DAYSENSORSCALED ");
  Serial.println(v,DEC);
*/


 v = amuxRead(VBATT1SCALED);
  Serial.print("VBATT1SCALED    ");
  voltage = v * VBATT1SCALE;
  Serial.println(voltage,2);



  v = amuxRead(VBATT2SCALED);
  Serial.print("VBATT2SCALED    ");
    voltage = v * VBATT2SCALE;
  Serial.println(voltage,2);

  v = amuxRead(VBATT3SCALED);
  Serial.print("VBATT3SCALED    ");
  voltage = v * VBATT3SCALE;
  Serial.println(voltage,2);


/*
  v = amuxRead(V3P3SCALED);
  Serial.print("V3P3SCALED      ");
  Serial.println(v,DEC);

  v = amuxRead(V5SWSCALED);
  Serial.print("V5SWSCALED      ");
  Serial.println(v,DEC);
*/

  Serial.println();
  delay(900);
}

