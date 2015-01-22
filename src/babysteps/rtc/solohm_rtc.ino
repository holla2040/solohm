#include <Wire.h>

/* from adafruit MCP4726 lib */
#define MCP4726_CMD_WRITEDAC            (0x40)  // Writes data to the DAC
#define MCP4726_CMD_WRITEDACEEPROM      (0x60)  // Writes data to the DAC and the EEPROM (persisting the assigned value after reset)


#define ADDRESS 0x60

uint16_t output;

void setup()
{
  Wire.begin();
  Serial.begin(115200);
  Serial.println("begin solohm_dac");
  output = 0;

  Wire.beginTransmission(ADDRESS);
  Wire.write(MCP4726_CMD_WRITEDACEEPROM);
  Wire.write(0x10);        // 1kohm to ground, 0 output                 
  Wire.write(0x00);           
  Wire.endTransmission();
}

void loop()
{ 
  Serial.println(output);

  Wire.beginTransmission(ADDRESS);
  Wire.write(MCP4726_CMD_WRITEDAC);
  Wire.write(output / 16);                   // Upper data bits          (D11.D10.D9.D8.D7.D6.D5.D4)
  Wire.write((output % 16) << 4);            // Lower data bits          (D3.D2.D1.D0.x.x.x.x)
  Wire.endTransmission();

  output += 500;
  if (output > 4000) {
    output = 0;
  }

  delay(10);
}

