#include "solohm.h"

#define VBATT1SCALE  4.136/3057.0
#define VBATT2SCALE  8.279/3351.0
#define VBATT3SCALE 12.413/3130.0
#define V3P3SCALE    3.32/3836.0
#define V5SCALE      5.00/3712.0
#define VPANELSCALE 25.44/2081.0
#define IPANELM     -0.0025316
#define IPANELB      9.6075949


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
  pinMode(V5SWENABLE,OUTPUT);
  digitalWrite(V5SWENABLE,1);

  Serial.println("solohm_adc begin");
}

void loop() {
  int adc;
  float value;

  adc = analogRead(VPANELSCALED);
  Serial.print("VPANELSCALED    ");
  value = adc * VPANELSCALE;
  Serial.println(value,2);

  adc = analogRead(IPANELSCALED);
  value = IPANELM * adc + IPANELB;
  Serial.print("IPANELSCALED    ");
  Serial.println(value,2);


  adc = amuxRead(DAYSENSORSCALED);
  Serial.print("DAYSENSORSCALED ");
  Serial.println(adc,DEC);


  adc = amuxRead(VBATT1SCALED);
  Serial.print("VBATT1SCALED    ");
  value = adc * VBATT1SCALE;
  Serial.println(value,2);

  adc = amuxRead(VBATT2SCALED);
  Serial.print("VBATT2SCALED    ");
  value = adc * VBATT2SCALE;
  Serial.println(value,2);

  adc = amuxRead(VBATT3SCALED);
  Serial.print("VBATT3SCALED    ");
  value = adc * VBATT3SCALE;
  Serial.println(value,2);

  adc = amuxRead(V3P3SCALED);
  Serial.print("V3P3SCALED      ");
  value = adc * V3P3SCALE;
  Serial.println(value,2);

  adc = amuxRead(V5SWSCALED);
  Serial.print("V5SWSCALED      ");
  value = adc * V5SCALE;
  Serial.println(value,2);

  Serial.println();
  delay(900);
}



