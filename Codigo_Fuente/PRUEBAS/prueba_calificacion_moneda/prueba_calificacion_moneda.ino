#include "HX711.h"
#include <math.h>   // para fabs()

// Pines del HX711 en el D1 mini
#define HX_DOUT  D5
#define HX_SCK   D6

HX711 balanza;

// Factor de conversión actual
const float FACTOR_CONVERSION = -13244.0;

// UMBRALES Y RANGOS (en gramos)

// Zona muerta alrededor de 0 para considerar "sin moneda"
const float UMBRAL_SIN_MONEDA = 0.80;  // |peso| < 0.8 g => sin moneda

// Rangos de monedas (en gramos)
const float MIN_10   = 0.80;
const float MAX_10   = 4.00;

const float MIN_50   = 5.00;
const float MAX_50   = 8.00;

const float MIN_100  = 8.50;
const float MAX_100  = 12.00;

// Umbrales para detectar aparición/desaparición de moneda
const float TH_COIN_APPEAR = 0.8;  // si sube por encima => posible moneda
const float TH_COIN_LOST   = 0.4;  // si baja por debajo => moneda retirada

// Estabilidad con moneda
const float STABILITY_DELTA          = 0.30;   // máx variación para considerar estable
const unsigned long STABILITY_WINDOW = 800;    // ms para ventana de estabilidad

// Auto-tare (cuando NO hay moneda) 

const unsigned long AUTO_TARE_INTERVAL_MS    = 60000; // mínimo 60s entre auto-tares
const unsigned long AUTO_TARE_MIN_STABLE_MS  = 10000; // al menos 10s en reposo antes de auto-tare

const unsigned long ZERO_WINDOW_MS          = 1200;   // ventana corta para medir estabilidad del cero
const float ZERO_STABILITY_DELTA            = 0.25;   // variación máx alrededor de 0 para considerarlo estable

// Variables de estado

bool coinPresent = false;               // true = ya clasificamos moneda actual
bool detectingCoin = false;             // true = estamos en ventana de detección/estabilidad con moneda

unsigned long detectionStartMillis = 0; // inicio de ventana para comprobar estabilidad (moneda)
float minPesoDetect = 0.0;
float maxPesoDetect = 0.0;

// Auto-tare: ventana de estabilidad cerca de 0
bool zeroWindowActive = false;
unsigned long zeroWindowStartMillis = 0;
float minPesoZero = 0.0;
float maxPesoZero = 0.0;

unsigned long emptySinceMillis   = 0;   // desde cuándo estamos en "sin moneda"
unsigned long lastAutoTareMillis = 0;


// Clasificación 

String clasificarMonedaPorPeso(float peso) {
  if (peso >= MIN_10 && peso <= MAX_10) {
    return "10 colones";
  }
  else if (peso >= MIN_50 && peso <= MAX_50) {
    return "50 colones";
  }
  else if (peso >= MIN_100 && peso <= MAX_100) {
    return "100 colones";
  }
  // fuera de todos los rangos conocidos
  return "";
}


// Setup

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("Prueba de Clasificacion de Monedas (evento + estabilidad + auto-tare mejorado)");

  balanza.begin(HX_DOUT, HX_SCK);

  Serial.println("Esperando a que el HX711 esté listo...");
  while (!balanza.is_ready()) {
    Serial.println("HX711 no listo...");
    delay(200);
  }

  Serial.println("Retira cualquier moneda de la balanza...");
  delay(2000);

  Serial.println("Haciendo tare inicial...");
  balanza.tare(15);
  Serial.println("Tare inicial listo.");

  balanza.set_scale(FACTOR_CONVERSION);

  unsigned long now = millis();
  emptySinceMillis   = now;
  lastAutoTareMillis = now;

  Serial.println("Balanza lista. Coloca monedas una por una.");
  Serial.println("Solo se imprimirá cuando la moneda este estable y clasificada.");
  Serial.println("------------------------------------------------------------");
}


// Loop principal

void loop() {
  if (!balanza.is_ready()) {
    delay(50);
    return;
  }

  float peso = balanza.get_units(5);  // promedio de 5 lecturas
  unsigned long now = millis();

  // AUTO-TARE EN REPOSO

  bool enZonaCero = (fabs(peso) < UMBRAL_SIN_MONEDA);

  if (!coinPresent && enZonaCero) {
    // Estamos sin moneda en la balanza

    // Si recién entramos a la zona de cero, inicializamos contadores
    if (emptySinceMillis == 0) {
      emptySinceMillis = now;
    }

    // Ventana para medir estabilidad del cero
    if (!zeroWindowActive) {
      zeroWindowActive = true;
      zeroWindowStartMillis = now;
      minPesoZero = peso;
      maxPesoZero = peso;
    } else {
      if (peso < minPesoZero) minPesoZero = peso;
      if (peso > maxPesoZero) maxPesoZero = peso;
    }

    // Chequeamos estabilidad del cero en ventana corta
    if (now - zeroWindowStartMillis >= ZERO_WINDOW_MS) {
      float deltaZero = maxPesoZero - minPesoZero;

      bool ceroEstable = (deltaZero <= ZERO_STABILITY_DELTA);

      bool suficienteTiempoReposo = (now - emptySinceMillis >= AUTO_TARE_MIN_STABLE_MS);
      bool suficienteTiempoDesdeUltimoTare = (now - lastAutoTareMillis >= AUTO_TARE_INTERVAL_MS);

      if (ceroEstable && suficienteTiempoReposo && suficienteTiempoDesdeUltimoTare) {
        balanza.tare(15);
        lastAutoTareMillis = now;
        emptySinceMillis   = now;   // reiniciamos tiempo de reposo

        Serial.println("[AUTO-TARE] Ajuste de cero realizado.");

        // Reiniciamos ventana de cero después del tare
        zeroWindowActive = false;
        zeroWindowStartMillis = now;
        minPesoZero = maxPesoZero = 0.0;
      } else {
        // Reiniciamos ventana corta para volver a evaluar estabilidad
        zeroWindowActive = false;
        zeroWindowStartMillis = 0;
        minPesoZero = maxPesoZero = peso;
      }
    }

  } else {
    // No estamos en zona de cero
    zeroWindowActive = false;
    zeroWindowStartMillis = 0;
    minPesoZero = maxPesoZero = peso;
    // si no hay moneda pero tampoco estamos bajo el umbral, no contamos como reposo
    if (!coinPresent && !enZonaCero) {
      emptySinceMillis = 0;
    }
  }


  // DETECCIÓN POR EVENTO (MONEDA)

  if (!coinPresent) {
    // Todavía no hemos clasificado moneda actual

    if (!detectingCoin) {
      // Estado base: esperando aparición de moneda
      if (peso > TH_COIN_APPEAR) {
        detectingCoin = true;
        detectionStartMillis = now;
        minPesoDetect = peso;
        maxPesoDetect = peso;
      }
    } else {
      // Estamos en ventana de detección/estabilidad con moneda

      // Si el peso cae por debajo del umbral de pérdida, cancelamos detección
      if (peso < TH_COIN_LOST) {
        detectingCoin = false;
      } else {
        // Actualizamos min y max durante la ventana
        if (peso < minPesoDetect) minPesoDetect = peso;
        if (peso > maxPesoDetect) maxPesoDetect = peso;

        // Verificamos si ya cumplimos ventana de estabilidad
        if (now - detectionStartMillis >= STABILITY_WINDOW) {
          float delta = maxPesoDetect - minPesoDetect;

          if (delta <= STABILITY_DELTA) {
            // Peso estable: tomamos el promedio entre min y max
            float pesoEstable = (minPesoDetect + maxPesoDetect) / 2.0;

            // Clasificamos
            String tipo = clasificarMonedaPorPeso(pesoEstable);

            if (tipo.length() > 0) {
              Serial.print("Peso estable = ");
              Serial.print(pesoEstable, 2);
              Serial.print(" g  ->  ");
              Serial.println(tipo);

              coinPresent = true;   // ya clasificamos esta moneda
            }
            // Salimos de detección (haya o no haya clasificado)
            detectingCoin = false;
          } else {
            // No está estable aún: reiniciamos ventana para seguir observando
            detectionStartMillis = now;
            minPesoDetect = peso;
            maxPesoDetect = peso;
          }
        }
      }
    }
  } else {
    // Ya clasificamos una moneda, esperamos a que sea retirada
    if (peso < TH_COIN_LOST) {
      coinPresent = false;
      detectingCoin = false;
      emptySinceMillis = now;  // empezamos a contar reposo otra vez
    }
  }

  delay(80);
}
