#include <HX711.h>
#include <math.h>  

static HX711 balanza;


// Factor de conversión calibrado 
const float BALANZA_FACTOR_CONVERSION = -9950;   

// Número de muestras usadas en cada lectura puntual dentro de la ventana
const int BALANZA_NUM_MUESTRAS = 3;

// Estabilidad de moneda
const float BAL_STABILITY_DELTA          = 0.30;   // máx variación (en gramos) para considerar estable
const unsigned long BAL_STABILITY_WINDOW = 1000;    // ventana de estabilidad en ms
const unsigned long BAL_MAX_TIEMPO_MEDICION = 3000; // timeout total de medición (ms)

// Zona muerta alrededor de 0 para considerar "sin moneda"
const float BAL_UMBRAL_SIN_MONEDA = 0.80;  // |peso| < 0.8 g => sin moneda/ruido

// 10 colones (viejas + nuevas): aprox 0.6–2.2 g → dejamos margen seguro
const float BAL_MIN_10   = 0.80;
const float BAL_MAX_10   = 4.00;

// 50 colones: medido ~6.5 g → margen amplio
const float BAL_MIN_50   = 5.00;
const float BAL_MAX_50   = 8.00;

// 100 colones: medido ~9.0–10.1 g
const float BAL_MIN_100  = 8.50;
const float BAL_MAX_100  = 12.00;


// Debe llamarse una sola vez desde setup(), con los pines DOUT y SCK:
//   balanza_init(PIN_BALANZA_DOUT, PIN_BALANZA_SCK);
void balanza_init(int pin_dout, int pin_sck) {
  balanza.begin(pin_dout, pin_sck);

  // Esperar a que el HX711 esté listo
  while (!balanza.is_ready()) {
    Serial.println("[BALANZA] HX711 no listo...");
    delay(100);
  }

  // Tare inicial (sin peso)
  balanza.tare(15);  // promedio de 15 lecturas para offset

  // Configurar el factor de escala
  balanza.set_scale(BALANZA_FACTOR_CONVERSION);

  Serial.println("[BALANZA] Inicializada (tare + factor de conversion).");
}


float balanza_leerGramos() {
  if (!balanza.is_ready()) {
    return 0.0;
  }
  float gramos = balanza.get_units(BALANZA_NUM_MUESTRAS);
  return gramos;
}


// Clasificación de moneda por peso
// Recibe un peso en gramos y devuelve:
//   0 -> sin moneda / desconocida
//   1 -> 10 colones
//   2 -> 50 colones
//   3 -> 100 colones
int balanza_clasificarMoneda(float gramos) {
  float peso = gramos;

  

  // Zona muerta: sin moneda o ruido
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

  // Fuera de todos los rangos conocidos
  return 0;
}


// Medición bloqueante con estabilidad
//   - Se llama SOLO cuando el sensor de moneda (D2) ya detectó una moneda.
//   - Bloquea hasta ~3 s máximo mientras:
//       - Lee varias veces el HX711,
//       - Forma una ventana de STABILITY_WINDOW ms,
//       - Calcula min y max,
//       - Si max-min <= BAL_STABILITY_DELTA => peso estable.
//   - Si se logra estabilidad:
//       - Devuelve true,
//       - Escribe en pesoEstable el peso promedio de la ventana,
//       - Escribe en tipoMoneda la clasificación (1,2,3 o 0).
//   - Si NO se logra estabilidad antes del timeout:
//       - Devuelve false,
//       - Devuelve igualmente el último peso medido en pesoEstable,
//       - Clasifica igual (por si quieres tomar una decisión basada en eso).
bool balanza_medirMonedaEstable(float &pesoEstable, int &tipoMoneda) {
  // Asegurarse de que el HX711 responde
  unsigned long inicioEspera = millis();
  while (!balanza.is_ready()) {
    if (millis() - inicioEspera > 500) {
      // No respondió en 500 ms
      pesoEstable = 0.0;
      tipoMoneda  = 0;
      return false;
    }
    delay(10);
  }

  unsigned long inicioMedicion   = millis();
  unsigned long inicioVentana    = millis();

  // Inicializamos min/max a la primera lectura válida
  float peso = balanza.get_units(BALANZA_NUM_MUESTRAS);
  float minPeso = peso;
  float maxPeso = peso;
  float ultimoPeso = peso;

  // Bucle principal de medición hasta que:
  //  - tengamos estabilidad, o
  //  - se agote el tiempo máximo.
  while (millis() - inicioMedicion < BAL_MAX_TIEMPO_MEDICION) {

    if (balanza.is_ready()) {
      peso = balanza.get_units(BALANZA_NUM_MUESTRAS);
      ultimoPeso = peso;

      if (peso < minPeso) minPeso = peso;
      if (peso > maxPeso) maxPeso = peso;
    }

    unsigned long ahora = millis();

    // ¿Ya cumplimos la ventana de estabilidad?
    if (ahora - inicioVentana >= BAL_STABILITY_WINDOW) {
      float delta = maxPeso - minPeso;

      if (fabs(delta) <= BAL_STABILITY_DELTA) {
        // Peso estable: usamos el promedio de la ventana
        pesoEstable = (minPeso + maxPeso) / 2.0f;
        tipoMoneda  = balanza_clasificarMoneda(pesoEstable);
        return true;
      } else {
        // No estable aún: reiniciamos ventana con la última lectura
        inicioVentana = ahora;
        minPeso = ultimoPeso;
        maxPeso = ultimoPeso;
      }
    }

    // Pequeño delay entre lecturas para no saturar
    delay(50);
  }

  // Si llegamos aquí, no se logró estabilidad en el tiempo máximo
  pesoEstable = ultimoPeso;
  tipoMoneda  = balanza_clasificarMoneda(pesoEstable);
  return false;
}
