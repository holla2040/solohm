#include "Energia.h"

#include "solohm.h"

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
