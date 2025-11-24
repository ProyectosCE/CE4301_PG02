#include "HX711.h"

// Pines del HX711 en el D1 mini
#define HX_DOUT  D5
#define HX_SCK   D6

HX711 balanza;


// Calculado para que 1 moneda de 100 colones ≈ 9 g
const float FACTOR_CONVERSION = -8403.0;   
 

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Iniciando prueba de balanza (HX711 - Rob Tillaart)...");

  balanza.begin(HX_DOUT, HX_SCK);  

  // Esperar a que haya datos
  Serial.println("Esperando a que el HX711 este listo...");
  while (!balanza.is_ready()) {
    Serial.println("HX711 no listo...");
    delay(200);
  }

  // TARE inicial (sin peso)
  Serial.println("Retira todo el peso de la balanza...");
  delay(2000);
  Serial.println("Realizando tare...");
  balanza.tare(15);   // promedio de 15 lecturas para offset
  Serial.println("Tare listo.");

  // Configurar factor de conversion (escala)
  balanza.set_scale(FACTOR_CONVERSION);

  Serial.println("Listo. Ahora puedes poner la moneda sobre la balanza.");
  Serial.println("Mostrando solo el peso en gramos:");
}

void loop() {
  if (balanza.is_ready()) {
    // Lee el peso ya corregido por offset y factor de escala
    float peso = balanza.get_units(10);  // promedio de 10 lecturas

    // Imprime SOLO el valor del peso (en gramos)
    Serial.println(peso, 2);   // 2 decimales
  }
  else {
    
  }

  delay(300);  // ~3 lecturas por segundo
}
