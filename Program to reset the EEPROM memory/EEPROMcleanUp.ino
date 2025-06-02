#include <EEPROM.h>

void setup() {
  // Reset all EEPROM bytes to 255 (default state)
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 255);
  }
}

void loop() {
  // Nothing needed here
}
