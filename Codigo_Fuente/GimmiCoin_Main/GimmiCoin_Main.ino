#include <Arduino.h>


#define PIN_SENSOR_MONEDA   D2     // Sensor de moneda (pull-down externo)
#define PIN_SERVO           D1     // Servo SG92R
#define PIN_MOTOR           D8     // Motor DC (transistor)

#define PIN_DF_RX           D4     // DFPlayer: RX del ESP
#define PIN_DF_TX           D3     // DFPlayer: TX del ESP (con resistencia 1k)

// Duraciones principales
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


  sensorMoneda_init();
  servo_init();
  motor_init();
  sonido_init();

  
}


void loop() {

  // Si se detecta una moneda física
  if (sensorMoneda_hayNuevaMoneda()) {

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

  delay(10);  // pequeño respiro
}
