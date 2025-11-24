#include <Arduino.h>


// Tiempo mínimo entre pulsos (en microsegundos) para evitar rebotes
const unsigned long SENSOR_DEBOUNCE_US = 80000;   // 80 ms

// Variables compartidas con la ISR
volatile bool          sensorMoneda_flagNuevaMoneda = false;
volatile uint32_t      sensorMoneda_contador = 0;
volatile unsigned long sensorMoneda_ultimoPulso = 0;


void IRAM_ATTR sensorMoneda_isr() {
  unsigned long ahora = micros();

  // Debounce por tiempo
  if (ahora - sensorMoneda_ultimoPulso > SENSOR_DEBOUNCE_US) {
    sensorMoneda_ultimoPulso = ahora;
    sensorMoneda_flagNuevaMoneda = true;
    sensorMoneda_contador++;
  }
}


void sensorMoneda_init() {
  pinMode(PIN_SENSOR_MONEDA, INPUT);   

  // Interrupción por flanco ascendente
  attachInterrupt(digitalPinToInterrupt(PIN_SENSOR_MONEDA),
                  sensorMoneda_isr,
                  RISING);

  Serial.println(F("[SensorMoneda] Inicializado (pull-down externo, interrupción RISING)"));
}


bool sensorMoneda_hayNuevaMoneda() {
  noInterrupts();
  bool hay = sensorMoneda_flagNuevaMoneda;
  if (hay) {
    sensorMoneda_flagNuevaMoneda = false;
  }
  interrupts();
  return hay;
}


uint32_t sensorMoneda_getConteo() {
  noInterrupts();
  uint32_t c = sensorMoneda_contador;
  interrupts();
  return c;
}
