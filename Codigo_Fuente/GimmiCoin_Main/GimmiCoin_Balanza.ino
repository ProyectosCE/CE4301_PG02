#include <HX711.h>
#include <math.h>   

static HX711 balanza;

// Factor de conversión calibrado 
const float BALANZA_FACTOR_CONVERSION = -13244.0;

// Número de muestras para promediar al leer
const int BALANZA_NUM_MUESTRAS = 10;

// Rangos de clasificación (en gramos)
// 10 colones: ~0.6 (viejas) hasta ~2.2 (nuevas/conmemorativas) 
const float BAL_MIN_10   = 0.30;
const float BAL_MAX_10   = 4.00;

// 50 colones: medido ~6.5 g 
const float BAL_MIN_50   = 5.00;
const float BAL_MAX_50   = 8.50;

// 100 colones: medido ~9.0–10.1 g
const float BAL_MIN_100  = 8.50;
const float BAL_MAX_100  = 12.00;

// Umbral para considerar "sin moneda" (ruido alrededor de cero)
const float BAL_UMBRAL_SIN_MONEDA = 0.30;


// Inicialización de la balanza
// Debe llamarse una sola vez desde setup(), con los pines DT (DOUT) y SCK
// por ejemplo: balanza_init(PIN_BALANZA_DOUT, PIN_BALANZA_SCK);
void balanza_init(int pin_dout, int pin_sck) {
  // Inicializa el HX711 en los pines indicados
  balanza.begin(pin_dout, pin_sck);

  // Esperar a que el HX711 esté listo
  while (!balanza.is_ready()) {
    Serial.println("HX711 no listo...");
    delay(100);
  }

  // Tare inicial (sin peso en la balanza)
  balanza.tare(15);  // promedio de 15 lecturas para offset

  // Configurar el factor de escala
  balanza.set_scale(BALANZA_FACTOR_CONVERSION);

   Serial.println("[BALANZA] Inicializada con factor de conversion.");
}


// Lectura de peso en gramos
// Devuelve el peso en gramos usando el factor de conversión calibrado.
// Usa un promedio de BALANZA_NUM_MUESTRAS lecturas.
// Si el HX711 no está listo, devuelve 0.0.
float balanza_leerGramos() {
  if (!balanza.is_ready()) {
    // HX711 no listo, devolvemos 0 por seguridad
    return 0.0;
  }

  // Lee el peso ya corregido por tare y factor de conversión
  float gramos = balanza.get_units(BALANZA_NUM_MUESTRAS);

  return gramos;
}


// Clasificación de moneda por peso
// Recibe un peso en gramos y devuelve:
//   0 -> sin moneda / desconocida
//   1 -> 10 colones
//   2 -> 50 colones
//   3 -> 100 colones
//
// Nota: esto NO hace lectura del HX711; solo clasifica un valor que le pase.
// Lo normal será:
//   float g = balanza_leerGramos();
//   int tipo = balanza_clasificarMoneda(g);
int balanza_clasificarMoneda(float gramos) {
  float peso = gramos;

  // Si por alguna razón llegó negativo, lo pasamos a positivo
  if (peso < 0) {
    peso = -peso;
  }

  // Sin moneda / ruido muy cerca de cero
  if (fabs(peso) < BAL_UMBRAL_SIN_MONEDA) {
    return 0;
  }

  // 10 colones
  if (peso >= BAL_MIN_10 && peso <= BAL_MAX_10) {
    return 1;
  }

  // 50 colones
  if (peso >= BAL_MIN_50 && peso <= BAL_MAX_50) {
    return 2;
  }

  // 100 colones
  if (peso >= BAL_MIN_100 && peso <= BAL_MAX_100) {
    return 3;
  }

  // Si no cae en ningún rango, la consideramos desconocida
  return 0;
}
