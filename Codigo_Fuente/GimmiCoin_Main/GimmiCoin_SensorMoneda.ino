#include <Arduino.h>


// Tiempo mínimo entre pulsos (en microsegundos) para evitar rebotes
const unsigned long SENSOR_DEBOUNCE_US = 80000;   // 80 ms

// Variables compartidas con la ISR
volatile bool          sensorMoneda_flagNuevaMoneda = false;
volatile uint32_t      sensorMoneda_contador = 0;
volatile unsigned long sensorMoneda_ultimoPulso = 0;


/* Function: sensorMoneda_isr
   Atiende la interrupción del sensor de moneda aplicando antirrebote.

   Params:
     - Ninguno.

   Returns:
     - void - no retorna valor.

   Restriction:
     Debe permanecer lo más corta posible y ejecutarse en contexto de ISR.
*/
void IRAM_ATTR sensorMoneda_isr() {
  unsigned long ahora = micros();

  // Debounce por tiempo
  if (ahora - sensorMoneda_ultimoPulso > SENSOR_DEBOUNCE_US) {
    sensorMoneda_ultimoPulso = ahora;
    sensorMoneda_flagNuevaMoneda = true;
    sensorMoneda_contador++;
  }
}


/* Function: sensorMoneda_init
   Configura el pin del sensor y registra la interrupción correspondiente.

   Params:
     - Ninguno.

   Returns:
     - void - no retorna valor.

   Restriction:
     Debe llamarse durante setup antes de detectar monedas.
*/
void sensorMoneda_init() {
  pinMode(PIN_SENSOR_MONEDA, INPUT);   

  // Interrupción por flanco ascendente
  attachInterrupt(digitalPinToInterrupt(PIN_SENSOR_MONEDA),
                  sensorMoneda_isr,
                  RISING);

  Serial.println(F("[SensorMoneda] Inicializado (pull-down externo, interrupción RISING)"));
}


/* Function: sensorMoneda_hayNuevaMoneda
   Consulta si la ISR ha registrado una nueva moneda desde la última lectura.

   Params:
     - Ninguno.

   Returns:
     - bool - true si existe una moneda pendiente de procesar.
*/
bool sensorMoneda_hayNuevaMoneda() {
  noInterrupts();
  bool hay = sensorMoneda_flagNuevaMoneda;
  if (hay) {
    sensorMoneda_flagNuevaMoneda = false;
  }
  interrupts();
  return hay;
}


/* Function: sensorMoneda_getConteo
   Devuelve el total acumulado de monedas detectadas por la ISR.

   Params:
     - Ninguno.

   Returns:
     - uint32_t - número de monedas contadas hasta el momento.
*/
uint32_t sensorMoneda_getConteo() {
  noInterrupts();
  uint32_t c = sensorMoneda_contador;
  interrupts();
  return c;
}


/* Function: sensorMoneda_setConteo
   Sincroniza el contador de la ISR con un valor proporcionado externamente.

   Params:
     - value: uint32_t - nuevo total que se desea reflejar en la ISR.

   Returns:
     - void - no retorna valor.

   Restriction:
     Debe ejecutarse con interrupciones deshabilitadas para evitar condiciones de carrera.
*/
void sensorMoneda_setConteo(uint32_t value) {
  noInterrupts();
  sensorMoneda_contador = value;
  sensorMoneda_flagNuevaMoneda = false;
  sensorMoneda_ultimoPulso = micros();
  interrupts();
}
