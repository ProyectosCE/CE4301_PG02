#include <Arduino.h>


#define PIN_SENSOR_MONEDA   D2     // Sensor de moneda (pull-down externo)
#define PIN_SERVO           D1     // Servo SG92R
#define PIN_MOTOR           D8     // Motor DC (transistor)

#define PIN_DF_RX           D4     // DFPlayer: RX del ESP
#define PIN_DF_TX           D3     // DFPlayer: TX del ESP (con resistencia 1k)

// Pines de la balanza (HX711)
#define PIN_BALANZA_DOUT    D5     // HX711 DT
#define PIN_BALANZA_SCK     D6     // HX711 SCK

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

// Motor 
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
  balanza_init(PIN_BALANZA_DOUT, PIN_BALANZA_SCK);
  
}


void loop() {

  // Si se detecta una moneda física por el sensor
  if (sensorMoneda_hayNuevaMoneda()) {

    Serial.println();
    Serial.print(F("[MAIN] Nueva moneda detectada. Conteo total = "));
    Serial.println(sensorMoneda_getConteo());

    // 1) Leer peso en gramos desde la balanza
    float gramos = balanza_leerGramos();
    Serial.print(F("[MAIN] Peso leído = "));
    Serial.print(gramos, 2);
    Serial.println(F(" g"));

    // 2) Clasificar la moneda según el peso
    int tipo = balanza_clasificarMoneda(gramos);

    // tipo:
    // 0 -> sin moneda / desconocida
    // 1 -> 10 colones
    // 2 -> 50 colones
    // 3 -> 100 colones

    if (tipo != 0) {
      // Moneda reconocida (10, 50 o 100)

      switch (tipo) {
        case 1: Serial.println(F("[MAIN] Moneda reconocida: 10 colones"));  break;
        case 2: Serial.println(F("[MAIN] Moneda reconocida: 50 colones"));  break;
        case 3: Serial.println(F("[MAIN] Moneda reconocida: 100 colones")); break;
        default: break;
      }

      // a) Reproducir audio
      sonido_reproMoneda();
      delay(T_SONIDO);  // dejamos sonar un rato

      // b) Servo: abrir y cerrar tapa
      servo_irAAbierto();
      delay(300);
      servo_irACerrado();
      delay(200);

      // c) Motor para retirar la moneda de la zona de pesaje
      motor_pulse();

    } else {
      // Moneda NO reconocida (fuera de rangos):
      Serial.println(F("[MAIN] Moneda NO reconocida. Solo se activa el motor para limpieza."));
      motor_pulse();
    }
  }

  delay(10);  // pequeño respiro
}
