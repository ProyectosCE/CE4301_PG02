// =====================================================================================
// Módulo de balanza (HX711) - Lectura y clasificación por rangos seguros
// =====================================================================================

#if defined(ARDUINO)
#include <Arduino.h>
#include <HX711.h>
#else
#include "arduino_stub.h"
#include <cstdlib>
// Stub para HX711 en modo desarrollo
class HX711 {
public:
  void begin(int, int) {}
  bool is_ready() { return true; }
  void tare(int) {}
  void set_scale(float) {}
  long read() { return static_cast<long>((std::rand() % 170001) - 120000); }
  long read_average(int) { return read(); }
  float get_units(int) { return read() / -13506.98407F; }
};
#endif

static HX711 balanza;

// Factor de conversión calibrado (mantener para compatibilidad legacy)
const float BALANZA_FACTOR_CONVERSION = -13506.98407F;

// Número de muestras usadas en cada lectura puntual dentro de la ventana
const int BALANZA_NUM_MUESTRAS = 3;

// Estabilidad de moneda (en unidades HX711)
const long BAL_STABILITY_DELTA = 6000;
const unsigned long BAL_STABILITY_WINDOW = 1000;
const unsigned long BAL_MAX_TIEMPO_MEDICION = 3000;

// Rangos "seguros" basados en la calibración detallada (valores en unidades HX711)
// Se prioriza evitar solapamientos: cualquier lectura fuera de estos rangos se marca desconocida.
const long RANGO_100_MIN = -180000;
const long RANGO_100_MAX = -63000;

const long RANGO_50_MIN  = -60000;
const long RANGO_50_MAX  = -10000;

const long RANGO_10_MIN  = -5000;
const long RANGO_10_MAX  = 20000;

int balanza_clasificarMonedaPorRangos(long unidades) {
  if (unidades >= RANGO_100_MIN && unidades <= RANGO_100_MAX) {
    Serial.println(F("[BALANZA] Clasificación: 100 colones (rango seguro)"));
    return 100;
  }

  if (unidades >= RANGO_50_MIN && unidades <= RANGO_50_MAX) {
    Serial.println(F("[BALANZA] Clasificación: 50 colones (rango seguro)"));
    return 50;
  }

  if (unidades >= RANGO_10_MIN && unidades <= RANGO_10_MAX) {
    Serial.println(F("[BALANZA] Clasificación: 10 colones (acero)"));
    return 10;
  }

  Serial.println(F("[BALANZA] Clasificación: desconocida (fuera de rangos seguros)"));
  return 0;
}

void balanza_init(int pin_dout, int pin_sck) {
  balanza.begin(pin_dout, pin_sck);

  // Esperar a que el HX711 esté listo antes de continuar
  while (!balanza.is_ready()) {
    Serial.println(F("[BALANZA] HX711 no listo..."));
    delay(100);
  }

  // Tare inicial (sin peso) - solo offset, sin factor de escala adicional
  balanza.tare(15);

  Serial.println(F("[BALANZA] Inicializada con tara. Trabajando con unidades directas."));
}

long balanza_leerUnidades() {
  if (!balanza.is_ready()) {
    return 0;
  }

  // get_units aplica la tara interna; mantenemos escala en 1 para medir en unidades crudas
  const float unidades = balanza.get_units(BALANZA_NUM_MUESTRAS);
  return static_cast<long>(unidades);
}

void balanza_realizarTara() {
  unsigned long esperaInicio = millis();
  while (!balanza.is_ready()) {
    if (millis() - esperaInicio > 500UL) {
      Serial.println(F("[BALANZA] No se pudo tarar: HX711 no responde."));
      return;
    }
    delay(20);
  }

  balanza.tare(15);
  Serial.println(F("[BALANZA] Tara ejecutada manualmente."));
}

bool balanza_medirMonedaEstable(float &pesoEstable, int &tipoMoneda) {
  // Asegurarse de que el HX711 responde antes de iniciar
  unsigned long inicioEspera = millis();
  while (!balanza.is_ready()) {
    if (millis() - inicioEspera > 500UL) {
      pesoEstable = 0.0F;
      tipoMoneda = 0;
      return false;
    }
    delay(10);
  }

  unsigned long inicioMedicion = millis();
  unsigned long inicioVentana = millis();

  long unidades = static_cast<long>(balanza.get_units(BALANZA_NUM_MUESTRAS));
  long minUnidades = unidades;
  long maxUnidades = unidades;
  long ultimasUnidades = unidades;

  while (millis() - inicioMedicion < BAL_MAX_TIEMPO_MEDICION) {
    if (balanza.is_ready()) {
      unidades = static_cast<long>(balanza.get_units(BALANZA_NUM_MUESTRAS));
      ultimasUnidades = unidades;

      if (unidades < minUnidades) minUnidades = unidades;
      if (unidades > maxUnidades) maxUnidades = unidades;
    }

    const unsigned long ahora = millis();
    if (ahora - inicioVentana >= BAL_STABILITY_WINDOW) {
      const long delta = maxUnidades - minUnidades;

      if (labs(delta) <= BAL_STABILITY_DELTA) {
        const long unidadesPromedio = (minUnidades + maxUnidades) / 2;
        pesoEstable = static_cast<float>(unidadesPromedio);
        tipoMoneda = balanza_clasificarMonedaPorRangos(unidadesPromedio);

        Serial.print(F("[BALANZA] Estable: "));
        Serial.print(unidadesPromedio);
        Serial.print(F(" unidades -> "));
        if (tipoMoneda > 0) {
          Serial.print(F("₡"));
          Serial.println(tipoMoneda);
        } else {
          Serial.println(F("desconocido"));
        }

        return true;
      }

      // No estable aún: reiniciar ventana de medición
      inicioVentana = ahora;
      minUnidades = ultimasUnidades;
      maxUnidades = ultimasUnidades;
    }

    delay(50);
  }

  // Si se agota el tiempo máximo, devolver la última lectura como referencia
  pesoEstable = static_cast<float>(ultimasUnidades);
  tipoMoneda = balanza_clasificarMonedaPorRangos(ultimasUnidades);

  Serial.print(F("[BALANZA] Timeout: "));
  Serial.print(ultimasUnidades);
  Serial.print(F(" unidades -> "));
  if (tipoMoneda > 0) {
    Serial.print(F("₡"));
    Serial.println(tipoMoneda);
  } else {
    Serial.println(F("desconocido"));
  }

  return false;
}
