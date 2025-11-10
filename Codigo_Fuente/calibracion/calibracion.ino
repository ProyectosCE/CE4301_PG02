#include "HX711.h"

// Pines
#define DOUT D5
#define CLK  D6

HX711 balanza;

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando HX711...");

  balanza.begin(DOUT, CLK);
  
  // Esperar un poco para estabilizar
  delay(1000);

  if (!balanza.is_ready()) {
    Serial.println("HX711 no está listo. Verifica conexiones.");
    while (1);
  }

  Serial.println("Coloca la balanza sin peso...");
  delay(3000);

  balanza.set_scale();    // sin factor de calibración aún
  balanza.tare();         // poner a cero la balanza
  Serial.println("Balanza puesta a cero.");
  Serial.println("Coloca un peso conocido para calibrar.");
}

void loop() {
  if (balanza.is_ready()) {
    long lectura = balanza.read_average(10);  // promedio de 10 lecturas
    Serial.print("Lectura sin calibrar: ");
    Serial.println(lectura);
  } else {
    Serial.println("HX711 no listo.");
  }

  delay(1000);
}
