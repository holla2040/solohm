#include "Energia.h"
#include <string.h>

#include "solohm.h"
#include "WiFi.h"

SolOhm::SolOhm() {

}

void SolOhm::setup() {
    Serial.begin(115200);
    Wire.begin();

// ADC setup
    pinMode(VPANELSCALED,INPUT);
    pinMode(IPANELSCALED,INPUT);
    pinMode(AMUXOUT,INPUT);

    pinMode(AMUXS0,OUTPUT);
    pinMode(AMUXS1,OUTPUT);
    pinMode(AMUXS2,OUTPUT);
    pinMode(V5SWENABLE,OUTPUT);
    digitalWrite(V5SWENABLE,1);

// DAC setup
    Wire.beginTransmission(DACADDRESS);
    Wire.write(MCP4726_CMD_WRITEDACEEPROM);
    Wire.write(0x10);        // 1kohm to ground, 0 output
    Wire.write(0x00);
    Wire.endTransmission();

// RTC setup
    setRTCRegister(DSRTCLib_SP, getRTCRegister(DSRTCLib_SP) & !DSRTCLib_SP_EOSC); //start RTC
    setRTCRegister(DSRTCLib_TR,0xA6); // no diode, 2k resistor;

    Serial.println("solohm setup");
}

void SolOhm::setRTCRegister(unsigned char registerNumber, unsigned char value) {
    Wire.beginTransmission(DSRTCLib_CTRL_ID);
    Wire.write(registerNumber); // set register pointer
    Wire.write(value);
    Wire.endTransmission();
}

uint8_t SolOhm::getRTCRegister(unsigned char registerNumber) {
    Wire.beginTransmission(DSRTCLib_CTRL_ID);
    Wire.write(registerNumber);
    Wire.endTransmission();
    Wire.requestFrom(DSRTCLib_CTRL_ID, 1);
    return Wire.read();
}

void SolOhm::setup(char *ssid, char *password) {
    setup(ssid,password,CONSOLEPORT);
}

void SolOhm::setup(char *ssid, char *password, uint16_t port) {
    setup();

    strcpy(wifiSSID,ssid);
    strcpy(wifiPassword,password);
    consolePort = port;
    Serial.print("wifi: connecting to AP ");
    Serial.println(wifiSSID);
    WiFi.begin(wifiSSID,wifiPassword);
    while ( WiFi.status() != WL_CONNECTED) {
        delay(300);
    }
    Serial.print("wifi: getting IP address ");
    while (WiFi.localIP() == INADDR_NONE) {
        delay(300);
    }
    Serial.println(WiFi.localIP());
    udp.begin(consolePort);
} 

// socat UDP4-RECVFROM:8380,broadcast,fork STDOUT
void SolOhm::udpBroadcast(char *s) {
    udp.beginPacket("255.255.255.255",consolePort+1);
    udp.print(s);
    udp.endPacket();
}

void SolOhm::dacSet(uint16_t v) {
    Wire.beginTransmission(DACADDRESS);
    Wire.write(MCP4726_CMD_WRITEDAC);
    Wire.write(v / 16);                   // Upper data bits          (D11.D10.D9.D8.D7.D6.D5.D4)
    Wire.write((v % 16) << 4);            // Lower data bits          (D3.D2.D1.D0.x.x.x.x)
    Wire.endTransmission();
}

uint16_t SolOhm::amuxRead(uint8_t channel) {
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

float SolOhm::dmmRead(uint8_t channel) {
    switch (channel) {
        case DAYSENSOR:      
            return amuxRead(DAYSENSORSCALED);
            break;
        case VBATT1:      
            return float(amuxRead(VBATT1SCALED))*VBATT1SCALE;
            break;
        case VBATT2:      
            return float(amuxRead(VBATT2SCALED))*VBATT2SCALE;
            break;
        case VBATT3:      
            return float(amuxRead(VBATT3SCALED))*VBATT3SCALE;
            break;
        case V3P3:      
            return float(amuxRead(V3P3SCALED))*V3P3SCALE;
            break;
        case V5:      
            return float(amuxRead(V5SWSCALED))*V5SCALE;
            break;
        case IPANEL:
            return IPANELM * float(analogRead(IPANELSCALED)) + IPANELB;
            break;
        case VPANEL:      
            return float(analogRead(VPANELSCALED))*VPANELSCALE;
            break;
    }
    return 0.0;
}
