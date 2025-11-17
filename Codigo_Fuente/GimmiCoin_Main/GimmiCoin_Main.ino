#include <Arduino.h>

// Pines principales
#define PIN_SENSOR_MONEDA D2     // sensor con pull-down externo
#define PIN_LED_MONEDA    D7     // LED indicador
#define PIN_SERVO         D1     // servo SG92R
#define PIN_MOTOR         D8     // transistor del motor DC

// Duración del LED encendido al detectar moneda
const int T_LED_MONEDA = 300;

// Prototipos del módulo sensor
void sensorMoneda_init();
bool sensorMoneda_hayNuevaMoneda();
uint32_t sensorMoneda_getConteo();

// Prototipos del módulo servo
void servo_init();
void servo_irAAbierto();
void servo_irACerrado();

// Prototipos del módulo motor
void motor_init();
void motor_pulse();


void setup() {
  pinMode(PIN_LED_MONEDA, OUTPUT);
  digitalWrite(PIN_LED_MONEDA, LOW);

  sensorMoneda_init();
  servo_init();
  motor_init();
}


void loop() {
  // Verificar si se detectó moneda
  if (sensorMoneda_hayNuevaMoneda()) {

    // LED indicador
    digitalWrite(PIN_LED_MONEDA, HIGH);
    delay(T_LED_MONEDA);
    digitalWrite(PIN_LED_MONEDA, LOW);

    // Secuencia del servo
    servo_irAAbierto();
    delay(300);
    servo_irACerrado();
    delay(200);

    // Activar motor DC
    motor_pulse();
  }

  delay(10);
}
