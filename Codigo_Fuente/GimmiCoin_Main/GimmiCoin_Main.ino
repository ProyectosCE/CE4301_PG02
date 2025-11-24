#include <Arduino.h>

#define PIN_SENSOR_MONEDA   D2     // Sensor de moneda (pull-down externo)
#define PIN_MOTOR           D8     // Motor DC (transistor)

#define PIN_DF_RX           D4     // DFPlayer: RX del ESP
#define PIN_DF_TX           D3     // DFPlayer: TX del ESP (con resistencia 1k)

// Pines de la balanza (HX711)
#define PIN_BALANZA_DOUT    D5     // HX711 DT
#define PIN_BALANZA_SCK     D6     // HX711 SCK

// Duraciones principales
const int T_SONIDO     = 2000;

// Sensor de moneda
void sensorMoneda_init();
bool sensorMoneda_hayNuevaMoneda();
uint32_t sensorMoneda_getConteo();

// Motor 
void motor_init();
void motor_pulse();

// Sonido
void sonido_init();
void sonido_reproMoneda();

// Balanza
void balanza_init(int pin_dout, int pin_sck);
bool balanza_medirMonedaEstable(float &pesoEstable, int &tipoMoneda);


void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println();
  Serial.println(F("GimmiCoin - Sistema Principal"));

  sensorMoneda_init();
  motor_init();
  sonido_init();
  balanza_init(PIN_BALANZA_DOUT, PIN_BALANZA_SCK);

  Serial.println(F("[MAIN] Inicialización completa. Esperando monedas..."));
}



void loop() {

  // Si se detecta una moneda física por el sensor
  if (sensorMoneda_hayNuevaMoneda()) {

    Serial.println();
    Serial.print(F("[MAIN] Nueva moneda detectada. Conteo total = "));
    Serial.println(sensorMoneda_getConteo());

    delay(1000);
    // MEDICIÓN CON ESTABILIDAD

    float peso = 0.0;
    int tipo  = 0;

    bool estable = balanza_medirMonedaEstable(peso, tipo);

    Serial.print(F("[MAIN] Peso estable medido = "));
    Serial.print(peso, 2);
    Serial.println(F(" g"));

    Serial.print(F("[MAIN] Tipo moneda detectada = "));
    Serial.println(tipo);

    if (!estable) {
      Serial.println(F("[MAIN] ADVERTENCIA: Peso NO fue completamente estable (timeout)."));
    }

    // ACCIONES SEGÚN TIPO DE MONEDA

    if (tipo != 0) {
      // Moneda reconocida (10, 50 o 100 colones)

      switch (tipo) {
        case 1: Serial.println(F("[MAIN] Moneda reconocida: 10 colones"));  break;
        case 2: Serial.println(F("[MAIN] Moneda reconocida: 50 colones"));  break;
        case 3: Serial.println(F("[MAIN] Moneda reconocida: 100 colones")); break;
      }

      // Reproducir audio 
      sonido_reproMoneda();
      delay(T_SONIDO);

      //Motor para retirar la moneda
      motor_pulse();

    } else {
      // Moneda NO reconocida
      Serial.println(F("[MAIN] Moneda NO reconocida. Solo se activa el motor."));
    }
  }

  delay(10);  // pequeño respiro
}
