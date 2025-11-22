#include "HX711.h"
#include <math.h>   // para fabs()

// Pines del HX711 en el D1 mini
#define HX_DOUT  D5
#define HX_SCK   D6

HX711 balanza;

// Factor de conversión actual
const float FACTOR_CONVERSION = -13244.0;

// UMBRALES Y RANGOS (en gramos)

// Umbral para considerar "sin moneda" (zona muerta alrededor de 0)
const float UMBRAL_SIN_MONEDA = 1.0;  // |peso| < 0.8 g => sin moneda

// Rangos de monedas
const float MIN_10   = 0.80;
const float MAX_10   = 4.00;

const float MIN_50   = 5.00;
const float MAX_50   = 8.00;

const float MIN_100  = 8.50;
const float MAX_100  = 12.00;

// Umbrales para detectar aparición/desaparición de moneda
const float TH_COIN_APPEAR = 0.8;  // si sube por encima => posible moneda
const float TH_COIN_LOST   = 0.4;  // si baja por debajo => moneda retirada

// Estabilidad
const float STABILITY_DELTA          = 0.30;   // máx variación para considerar estable
const unsigned long STABILITY_WINDOW = 800;    // ms para ventana de estabilidad

// Auto-tare
const unsigned long AUTO_TARE_INTERVAL_MS    = 60000; // mínimo 60s entre auto-tares
const unsigned long AUTO_TARE_MIN_STABLE_MS  = 10000; // al menos 10s sin moneda y estable

// ---------- Variables de estado ----------

bool coinPresent = false;               // true = ya clasificamos moneda actual
bool detectingCoin = false;             // true = estamos en ventana de detección/estabilidad

unsigned long detectionStartMillis = 0; // inicio de ventana para comprobar estabilidad

float minPesoDetect = 0.0;              // min peso visto durante la ventana
float maxPesoDetect = 0.0;              // max peso visto durante la ventana

unsigned long lastStableNoCoinMillis = 0;
unsigned long lastAutoTareMillis     = 0;


//Clasificación

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


//Setup 

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("=== Prueba de Clasificacion de Monedas (evento + estabilidad + auto-tare) ===");

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
  lastStableNoCoinMillis = now;
  lastAutoTareMillis     = now;

  Serial.println("Balanza lista. Coloca monedas una por una.");
  Serial.println("Solo se imprimirá cuando la moneda este estable y clasificada.");
  Serial.println("------------------------------------------------------------");
}


//Loop principal

void loop() {
  if (!balanza.is_ready()) {
    // Si el HX711 no está listo, no se hace nada
    delay(50);
    return;
  }

  // Lectura promediada
  float peso = balanza.get_units(5);  // promedio de 5 lecturas

  unsigned long now = millis();

  // Auto-tare en reposo
  // Solo cuando NO hay moneda, el peso está cerca de 0 y ha estado así un buen rato
  if (!coinPresent && fabs(peso) < UMBRAL_SIN_MONEDA) {
    // actualiza tiempo de estabilidad sin moneda
    lastStableNoCoinMillis = now;

    if ((now - lastAutoTareMillis > AUTO_TARE_INTERVAL_MS) &&
        (now - lastStableNoCoinMillis > AUTO_TARE_MIN_STABLE_MS)) {

      // Hace un pequeño tare para corregir drift
      balanza.tare(15);
      lastAutoTareMillis = now;

      //  mensaje de depuración
      Serial.println("[AUTO-TARE] Ajuste de cero realizado.");
    }
  }

  //Lógica de detección por evento 
  if (!coinPresent) {
    if (!detectingCoin) {
      // Estado base: esperando aparición de moneda
      if (peso > TH_COIN_APPEAR) {
        // Empezamos ventana de detección/estabilidad
        detectingCoin = true;
        detectionStartMillis = now;
        minPesoDetect = peso;
        maxPesoDetect = peso;
      }
      // Si peso <= TH_COIN_APPEAR, seguir esperando sin hacer nada
    } 
    else {
      // ventana de detección: moneda parece estar presente

      // Si el peso cae por debajo del umbral de pérdida, cancelar detección
      if (peso < TH_COIN_LOST) {
        detectingCoin = false;
      } 
      else {
        // Actualizar min y max durante la ventana
        if (peso < minPesoDetect) minPesoDetect = peso;
        if (peso > maxPesoDetect) maxPesoDetect = peso;

        // Verificamos si ya cumplió ventana de estabilidad
        if (now - detectionStartMillis >= STABILITY_WINDOW) {
          float delta = maxPesoDetect - minPesoDetect;

          if (delta <= STABILITY_DELTA) {
            // Peso estable: tomar el promedio entre min y max como peso estable
            float pesoEstable = (minPesoDetect + maxPesoDetect) / 2.0;

            // Clasificar
            String tipo = clasificarMonedaPorPeso(pesoEstable);

            if (tipo.length() > 0) {
              Serial.print("Peso estable = ");
              Serial.print(pesoEstable, 2);
              Serial.print(" g  ->  ");
              Serial.println(tipo);

              coinPresent = true;
            }
            detectingCoin = false;
          }
          else {
            // Si no está estable aún, extender la ventana:
            // reiniciar tiempos y min/max para seguir observando
            detectionStartMillis = now;
            minPesoDetect = peso;
            maxPesoDetect = peso;
          }
        }
      }
    }
  } 
  else {
    // Ya clasificada una moneda, esperar a que sea retirada

    if (peso < TH_COIN_LOST) {
      // Moneda retirada: volvemos al estado base
      coinPresent = false;
      detectingCoin = false;
      lastStableNoCoinMillis = now;
    }
  }

  // Pequeño delay para no saturar
  delay(80);
}
