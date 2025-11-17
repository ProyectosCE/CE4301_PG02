#include <Arduino.h>
#include <HX711.h>

// Usa los pines que tenés ahora:
const int PIN_BALANZA_DOUT = D5;   // DT
const int PIN_BALANZA_SCK  = D6;   // SCK

HX711 balanza;

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Iniciando HX711...");

  balanza.begin(PIN_BALANZA_DOUT, PIN_BALANZA_SCK);

  // Sin escala ni tare todavía
}

void loop() {
  if (balanza.is_ready()) {
    long raw = balanza.read();  // valor crudo del ADC
    Serial.print("Raw: ");
    Serial.println(raw);
  } else {
    Serial.println("HX711 no listo...");
  }

  delay(300);
}
