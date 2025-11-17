#include <Arduino.h>


#define PIN_SENSOR_MONEDA   D2     // Sensor de moneda (pull-down externo)
#define PIN_LED_MONEDA      D7     // LED indicador
#define PIN_SERVO           D1     // Servo SG92R
#define PIN_MOTOR           D8     // Motor DC (transistor)

#define PIN_DF_RX           D3     // DFPlayer: RX del ESP
#define PIN_DF_TX           D4     // DFPlayer: TX del ESP (con resistencia 1k)

#define PIN_BALANZA_DOUT    D5     // HX711 DT
#define PIN_BALANZA_SCK     D6     // HX711 SCK

// Duraciones principales
const int T_LED_MONEDA = 300;
const int T_SONIDO     = 1400;


// Sensor de moneda
void sensorMoneda_init();
bool sensorMoneda_hayNuevaMoneda();
uint32_t sensorMoneda_getConteo();

// Servo
void servo_init();
void servo_irAAbierto();
void servo_irACerrado();

// Motor (versión con delay)
void motor_init();
void motor_pulse();

// Sonido
void sonido_init();
void sonido_reproMoneda();

// Balanza
void balanza_init(int pin_dout, int pin_sck);
float balanza_leerGramos();
int balanza_clasificarMoneda(float gramos);


void setup() {

  pinMode(PIN_LED_MONEDA, OUTPUT);
  digitalWrite(PIN_LED_MONEDA, LOW);

  sensorMoneda_init();
  servo_init();
  motor_init();
  sonido_init();

  // Inicialización de la balanza HX711 con pines definidos aquí
  balanza_init(PIN_BALANZA_DOUT, PIN_BALANZA_SCK);
}


void loop() {

  // Si se detecta una moneda física
  if (sensorMoneda_hayNuevaMoneda()) {

    digitalWrite(PIN_LED_MONEDA, HIGH);
    delay(T_LED_MONEDA);
    digitalWrite(PIN_LED_MONEDA, LOW);

    float gramos = balanza_leerGramos();
    int tipo = balanza_clasificarMoneda(gramos);

    
    if (tipo != 0) {

      // Sonido
      sonido_reproMoneda();
      delay(T_SONIDO);

      // Servo abrir/cerrar
      servo_irAAbierto();
      delay(300);
      servo_irACerrado();
      delay(200);

      // Motor para retirar la moneda
      motor_pulse();
    }

    
    else {
      // Sin sonido, sin servo, sin motor
      // Moneda se queda en el área hasta timeout (FSM futura)
    }
  }

  delay(10);  // pequeño respiro
}
