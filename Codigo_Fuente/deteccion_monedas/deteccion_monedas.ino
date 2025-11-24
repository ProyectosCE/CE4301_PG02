#include "HX711.h"

#define DOUT D5
#define CLK  D6

HX711 balanza;

// 🔹 Factor corregido (ahora positivo)
float factor_calibracion = 15620.0;

// Umbral para detectar cambio de peso (en gramos)
const float UMBRAL_DETECTAR = 0.5;
const int PROMEDIOS = 10;

bool esperando_moneda = true;
float peso_base = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("\nIniciando detector de monedas...");

  balanza.begin(DOUT, CLK);
  balanza.set_scale(factor_calibracion);
  balanza.tare();
  delay(1500);

  peso_base = fabs(balanza.get_units(PROMEDIOS));
  Serial.print("Peso base inicial: ");
  Serial.println(peso_base, 3);

  Serial.println("Listo para detectar monedas.\n");
}

void loop() {
  float peso_actual = fabs(balanza.get_units(PROMEDIOS));  // usar valor absoluto

  if (esperando_moneda) {
    if (peso_actual - peso_base > UMBRAL_DETECTAR) {
      Serial.println("\nMoneda detectada, esperando estabilización...");
      delay(1800);

      float peso_estable = fabs(promedioEstable(25));  // valor positivo

      Serial.print("Peso detectado: ");
      Serial.print(peso_estable, 2);
      Serial.println(" g");

      String tipo = detectarMoneda(peso_estable);
      Serial.print("→ ");
      Serial.println(tipo);

      esperando_moneda = false;
    }
  } else {
    // Esperar que se retire la moneda (peso bajo)
    if (peso_actual < UMBRAL_DETECTAR) {
      peso_base = promedioEstable(15);
      esperando_moneda = true;
      Serial.println("\nListo para la siguiente moneda.");
    }
  }

  delay(150);
}

// --- Clasificación de monedas ---
String detectarMoneda(float peso) {
  if (peso > 0.6 && peso < 2.0)  return "Moneda de ₡10";
  if (peso > 6.0 && peso < 7.5)  return "Moneda de ₡25";
  if (peso > 7.57 && peso < 8.2) return "Moneda de ₡50";
  if (peso > 8.2 && peso < 9.5)  return "Moneda de ₡100";
  return "Moneda desconocida";
}

// --- Suavizado de lectura ---
float promedioEstable(int muestras) {
  float suma = 0;
  for (int i = 0; i < muestras; i++) {
    suma += balanza.get_units(1);
    delay(30);  // evita WDT reset
  }
  return suma / muestras;
}
