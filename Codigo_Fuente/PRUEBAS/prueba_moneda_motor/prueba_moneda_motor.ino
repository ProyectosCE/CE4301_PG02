#include <Arduino.h>

// Pines
#define PIN_SENSOR_MONEDA  D2     // entrada con pull-down externo a GND
#define PIN_MOTOR          D8     // salida al transistor del motor

// Duración del pulso del motor (ms)
const int T_MOTOR = 2000;  

// Variable para detectar flanco
bool monedaAnterior = false;

void setup() {
  pinMode(PIN_SENSOR_MONEDA, INPUT);   
  pinMode(PIN_MOTOR, OUTPUT);
  digitalWrite(PIN_MOTOR, LOW);

  // Pequeña espera para estabilidad del pin al arrancar
  delay(50);

  // El estado inicial del pin se toma como "estado anterior"
  monedaAnterior = (digitalRead(PIN_SENSOR_MONEDA) == HIGH);
}

void loop() {

  // Detecta si hay moneda actualmente
  bool monedaActual = (digitalRead(PIN_SENSOR_MONEDA) == HIGH);

  // Detecta el flanco: de no-moneda → moneda
  if (monedaActual && !monedaAnterior) {

    // Activar motor
    digitalWrite(PIN_MOTOR, HIGH);
    delay(T_MOTOR);
    digitalWrite(PIN_MOTOR, LOW);

    delay(50); // evitar rebotes fuertes
  }

  // Actualizar estado anterior
  monedaAnterior = monedaActual;

  delay(5);
}
