#include <EEPROM.h>

const int memDir = 0; // misma dirección donde guardas "veces"

void setup() {
  Serial.begin(115200);

  EEPROM.begin(20);  
  int resetValue = 0;

  EEPROM.put(memDir, resetValue); // guardamos un 0
  EEPROM.commit(); // guardamos cambios
  Serial.println("EEPROM reseteada a 0 ✅");
}

void loop() {
  // vacío
}
