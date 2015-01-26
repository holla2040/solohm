#include <Wire.h>

/* from adafruit MCP4726 lib */
#define MCP4726_CMD_WRITEDAC            (0x40)  // Writes data to the DAC
#define MCP4726_CMD_WRITEDACEEPROM      (0x60)  // Writes data to the DAC and the EEPROM (persisting the assigned value after reset)


#define ADDRESS 0x60

enum {
  MODE_ZERO,MODE_CYCLE,MODE_MANUAL};

uint16_t output;
uint8_t mode,increment;
uint32_t timeout;


void setup()
{
  Wire.begin();
  Serial.begin(115200);
  output = 0;
  mode = MODE_ZERO;
  timeout = 1000;
  increment = 5;

  Wire.beginTransmission(ADDRESS);
  Wire.write(MCP4726_CMD_WRITEDACEEPROM);
  Wire.write(0x10);        // 1kohm to ground, 0 output                 
  Wire.write(0x00);           
  Wire.endTransmission();

  Serial.println("begin solohm_dac");

}

void dacSet(uint16_t v) {
  Wire.beginTransmission(ADDRESS);
  Wire.write(MCP4726_CMD_WRITEDAC);
  Wire.write(v / 16);                   // Upper data bits          (D11.D10.D9.D8.D7.D6.D5.D4)
  Wire.write((v % 16) << 4);            // Lower data bits          (D3.D2.D1.D0.x.x.x.x)
  Wire.endTransmission();
}

void loop()
{ 
  int c = 0;

  if (millis() > timeout) {
    Serial.print(millis(),DEC);
    Serial.print(" ");
    Serial.print(mode,DEC);
    Serial.print(" ");
    Serial.print(increment,DEC);
    Serial.print(" ");

    Serial.println(output);
    timeout = millis() + 1000;
  }
  if (Serial.available()) {
    c = Serial.read();
    switch (c) {
    case 'c':
      mode = MODE_CYCLE;
      break;
    case ' ':
      mode = MODE_ZERO;
      break;
    case '1':
      increment = 1;
      break;
    case '5': 
      increment = 5;
      break;
    case '0':
      increment = 10;
      break;
    default:
      mode = MODE_MANUAL;
    }
    timeout = 0;
  }

  if (mode == MODE_MANUAL){
    if (c == 'i') {
      if (output < 4000) {
        output += increment;
      }
    }

    if (c == 'k') {
      if (output > 0) {
        output -= increment;
      }
    }
    dacSet(output); 
    return;
  }
  if (mode == MODE_ZERO) {
    output = 0;
    dacSet(output); 
    return;
  }

  if (mode == MODE_CYCLE) {

    dacSet(output);

    output += 50;
    if (output > 4000) {
      output = 0;
      delay(1000);
    }

    delay(100);
    return;
  }


}











