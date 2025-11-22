#include "HX711.h"

// Pines del HX711 en el D1 mini
#define HX_DOUT  D5
#define HX_SCK   D6

HX711 balanza;

// Factor de conversión actual 
const float FACTOR_CONVERSION = -13244.0;

// Umbral para considerar "sin moneda"
const float UMBRAL_SIN_MONEDA = 0.99;  // |peso| menor a esto = sin moneda

// Rangos de clasificación (en gramos)
const float MIN_10   = 1.00;
const float MAX_10   = 4.00;

const float MIN_50   = 5.00;
const float MAX_50   = 8.00;

const float MIN_100  = 8.50;
const float MAX_100  = 12.00;

// Devuelve nombre de la moneda o cadena vacía si no coincide con ninguna
String clasificarMoneda(float peso) {
  // Sin moneda o ruido muy cerca de cero
  if (fabs(peso) < UMBRAL_SIN_MONEDA) {
    return "";  // no hay moneda
  }

  if (peso >= MIN_10 && peso <= MAX_10) {
    return "10 colones";
  }
  else if (peso >= MIN_50 && peso <= MAX_50) {
    return "50 colones";
  }
  else if (peso >= MIN_100 && peso <= MAX_100) {
    return "100 colones";
  }

  // Peso fuera de todos los rangos conocidos -> no imprimir nada
  return "";
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("Prueba de Clasificacion de Monedas");

  balanza.begin(HX_DOUT, HX_SCK);

  Serial.println("Esperando a que el HX711 esté listo...");
  while (!balanza.is_ready()) {
    Serial.println("HX711 no listo...");
    delay(200);
  }

  Serial.println("Retira cualquier moneda de la balanza...");
  delay(2000);

  Serial.println("Haciendo tare...");
  balanza.tare(15);
  Serial.println("Tare listo.");

  balanza.set_scale(FACTOR_CONVERSION);
  Serial.println("Balanza lista. Coloca una moneda.");
}

void loop() {
  if (balanza.is_ready()) {
    float peso = balanza.get_units(10);  // promedio de 10 lecturas

    String tipo = clasificarMoneda(peso);

    // Si no coincide con ninguna moneda (incluye sin moneda), no imprime nada
    if (tipo.length() > 0) {
      Serial.print("Peso = ");
      Serial.print(peso, 2);
      Serial.print(" g  →  ");
      Serial.println(tipo);
    }
  }

  delay(2000);
}
