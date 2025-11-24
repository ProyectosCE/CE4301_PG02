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

// Rangos aproximados medidos para cada denominación (en gramos)
//   0.55 – 2.30  -> ₡10 (monedas livianas)
//   2.30 – 4.52  -> ₡10 (monedas pesadas)
//   4.52 – 6.27  -> ₡50
//   6.27 – 8.00  -> ₡25
//   8.00 – 9.75  -> ₡100
//   9.75 – 12.0  -> ₡500


// Debe llamarse una sola vez desde setup(), con los pines DOUT y SCK:
//   balanza_init(PIN_BALANZA_DOUT, PIN_BALANZA_SCK);
/* Function: balanza_init
   Inicializa el módulo HX711 y realiza la tara y factor de escala.

   Params:
     - pin_dout: int - pin digital conectado a la salida de datos del HX711.
     - pin_sck: int - pin digital conectado al reloj del HX711.

   Returns:
     - void - no retorna valor.

   Restriction:
     Debe invocarse con la balanza sin carga para calibrar correctamente.
*/
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


/* Function: balanza_leerGramos
   Obtiene una lectura rápida de la balanza expresada en gramos.

   Params:
     - Ninguno.

   Returns:
     - float - peso estimado en gramos.
*/
float balanza_leerGramos() {
  if (!balanza.is_ready()) {
    return 0.0;
  }
  float gramos = balanza.get_units(BALANZA_NUM_MUESTRAS);
  return gramos;
}


// Clasificación de moneda por peso
// Devuelve el valor nominal en colones o -1 si no coincide con ningún rango conocido.
/* Function: balanza_clasificarMoneda
   Determina la denominación en colones de una moneda según su peso.

   Params:
     - gramos: float - peso de la moneda en gramos.

   Returns:
     - int - valor nominal en colones o -1 si el peso no corresponde a una moneda válida.
*/
int balanza_clasificarMoneda(float gramos) {
  if (!isfinite(gramos) || gramos <= 0.0F) {
    return -1;
  }

  const float w = gramos;

  if (w >= 0.55F && w < 2.30F) {
    return 10;
  }
  if (w >= 2.30F && w < 4.52F) {
    return 10;
  }
  if (w >= 4.52F && w < 6.27F) {
    return 50;
  }
  if (w >= 6.27F && w < 8.00F) {
    return 25;
  }
  if (w >= 8.00F && w < 9.75F) {
    return 100;
  }
  if (w >= 9.75F && w < 12.0F) {
    return 500;
  }

  return -1;
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
//       - Escribe en tipoMoneda el valor nominal detectado (₡10, ₡25, ₡50, ₡100, ₡500) o -1 si es inválido.
//   - Si NO se logra estabilidad antes del timeout:
//       - Devuelve false,
//       - Devuelve igualmente el último peso medido en pesoEstable,
//       - Clasifica igual (por si quieres tomar una decisión basada en eso).
/* Function: balanza_medirMonedaEstable
   Realiza lecturas sucesivas hasta obtener un peso estable para clasificar la moneda.

   Params:
     - pesoEstable: float& - referencia donde se almacena el peso final estimado.
     - tipoMoneda: int& - referencia para el código de la moneda detectada.

   Returns:
     - bool - true si se alcanzó la estabilidad dentro del tiempo límite.

   Restriction:
     Su ejecución es bloqueante y no debe interrumpirse mientras se mide.
*/
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
