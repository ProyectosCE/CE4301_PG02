#include <Arduino.h>

// Pines principales
#define PIN_SENSOR_MONEDA D2     // sensor con pull-down externo
#define PIN_LED_MONEDA    D7     // LED indicador
#define PIN_SERVO         D1     // servo SG92R
#define PIN_MOTOR         D8     // transistor del motor DC

// Pines del sonido
#define PIN_DF_RX D3            // RX del ESP8266 (recibe del DFPlayer)
#define PIN_DF_TX D4            // TX del ESP8266 (resistencia 1k al DFPlayer)

// Duración del LED encendido al detectar moneda
const int T_LED_MONEDA = 300;
const int T_SONIDO = 1400;      // tiempo aproximado del sonido

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

// Prototipos del módulo sonido
void sonido_init();
void sonido_reproMoneda();


void setup() {
  pinMode(PIN_LED_MONEDA, OUTPUT);
  digitalWrite(PIN_LED_MONEDA, LOW);

  sensorMoneda_init();
  servo_init();
  motor_init();
  sonido_init();
}


void loop() {
  // Verificar si se detectó moneda
  if (sensorMoneda_hayNuevaMoneda()) {

    // LED indicador
    digitalWrite(PIN_LED_MONEDA, HIGH);
    delay(T_LED_MONEDA);
    digitalWrite(PIN_LED_MONEDA, LOW);

    // Reproducir sonido
    sonido_reproMoneda();
    delay(T_SONIDO);

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
