#include <HX711.h>

HX711 balanza;

// Factor de calibración tomado de tu código
float factor_calibracion = 15620.0;

// Lectura suavizada (similar a promedioEstable)
static float balanza_promedioEstable(int muestras) {
  float suma = 0;
  for (int i = 0; i < muestras; i++) {
    suma += balanza.get_units(1);  // ya usa factor de calibración
    delay(30);                     // evita WDT reset
  }
  return suma / muestras;
}

// Inicialización de la balanza con pines definidos en el main
void balanza_init(int pin_dout, int pin_sck) {
  balanza.begin(pin_dout, pin_sck);
  balanza.set_scale(factor_calibracion);
  balanza.tare();
  delay(1500);  // estabilización
}

// Lectura filtrada en gramos
float balanza_leerGramos() {
  float gramos = balanza_promedioEstable(10);
  if (gramos < 0) gramos = -gramos;
  return gramos;
}

// Clasificación según rangos reales de monedas
int balanza_clasificarMoneda(float peso) {
  if (peso > 0.6 && peso < 2.0)  return 1;  // ₡10
  if (peso > 6.0 && peso < 7.5)  return 2;  // ₡25
  if (peso > 7.57 && peso < 8.2) return 3;  // ₡50
  if (peso > 8.2 && peso < 9.5)  return 4;  // ₡100
  return 0;
}
