#include "solohm.h"
#include <Wire.h> 
#include <WiFi.h>
// library hack, http://stackoverflow.com/questions/6504211/is-it-possible-to-include-a-library-from-another-library-using-the-arduino-ide

SolOhm solohm;
void setup() { 
  solohm.setup("energia","supersecret");
}

void loop() {
  solohm.loop();
}

