#if __has_include(<Arduino.h>)
#include <Arduino.h>
#else
#include "arduino_stub.h"
#endif

#if __has_include(<ESP8266WiFi.h>)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#else
#include "esp8266_stubs.h"
#endif

#if __has_include(<EEPROM.h>)
#include <EEPROM.h>
#else
#include "eeprom_stub.h"
#endif

#define PIN_SENSOR_MONEDA   D2     // Sensor de moneda (pull-down externo)
#define PIN_MOTOR           D8     // Motor DC (transistor)

#define PIN_DF_RX           D4     // DFPlayer: RX del ESP
#define PIN_DF_TX           D3     // DFPlayer: TX del ESP (con resistencia 1k)

// Pines de la balanza (HX711)
#define PIN_BALANZA_DOUT    D5     // HX711 DT
#define PIN_BALANZA_SCK     D6     // HX711 SCK

// Duraciones principales
const int T_SONIDO     = 2000;

// Configuración de red y autenticación web
const char* WIFI_AP_SSID     = "GimmiCoin";
const char* WIFI_AP_PASSWORD = "12345678";
const char* WEB_ADMIN_USER   = "admin";
const char* WEB_ADMIN_PASS   = "1234";

ESP8266WebServer server(80);

// Persistencia del contador en EEPROM
constexpr uint16_t EEPROM_BYTES            = 16;
constexpr int      EEPROM_ADDR_COUNTER     = 0;
constexpr uint8_t  EEPROM_WRITE_THRESHOLD  = 10;       // Guardar cada 10 monedas
constexpr unsigned long EEPROM_WRITE_DELAY = 30000UL;  // O tras 30 s sin nuevas monedas

static uint32_t coinCounter = 0;           // Conteo total persistente
static uint32_t lastPersistedCounter = 0;  // Último valor guardado en EEPROM
static uint32_t coinsSincePersist = 0;     // Conteo desde el último guardado
static bool     eepromDirty = false;       // Indica si hay datos pendientes
static unsigned long lastCoinMillis = 0;   // Última moneda detectada
bool webReady = false;

// Declaraciones adelantadas para utilitarios web/persistencia
void startWebServer(const char* ssid, const char* password);
void pumpServer();
void waitWithServer(unsigned long ms);
void manageEepromCommit();
void persistCounter(const char* reason);
uint32_t getCoinCounter();
void resetCoinCounter();
bool isCoinSensorActive();
void loadCounterFromEEPROM();

// Declaraciones adelantadas de módulos hardware
void sensorMoneda_setConteo(uint32_t value);

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


/* Function: setup
   Configura el hardware, restaura el contador persistente e inicia el servidor web.

   Params:
     - Ninguno.

   Returns:
     - void - no retorna valor.

   Restriction:
     Debe ejecutarse una sola vez al arranque del microcontrolador.
*/
void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println();
  Serial.println(F("GimmiCoin - Sistema Principal (con interfaz web)"));

  EEPROM.begin(EEPROM_BYTES);
  loadCounterFromEEPROM();

  sensorMoneda_init();
  sensorMoneda_setConteo(coinCounter);
  motor_init();
  sonido_init();
  balanza_init(PIN_BALANZA_DOUT, PIN_BALANZA_SCK);

  startWebServer(WIFI_AP_SSID, WIFI_AP_PASSWORD);

  Serial.println(F("[MAIN] Inicialización completa. Esperando monedas..."));
}



/* Function: loop
   Ejecuta el ciclo principal: atiende el servidor, mide monedas y coordina actuadores.

   Params:
     - Ninguno.

   Returns:
     - void - no retorna valor.

   Restriction:
     Debe ejecutarse continuamente y no debe bloquear por largos periodos.
*/
void loop() {
  pumpServer();
  manageEepromCommit();

  // Si se detecta una moneda física por el sensor
  if (sensorMoneda_hayNuevaMoneda()) {

    coinCounter = sensorMoneda_getConteo();
    coinsSincePersist = coinCounter - lastPersistedCounter;
    eepromDirty = true;
    lastCoinMillis = millis();

    Serial.println();
    Serial.print(F("[MAIN] Nueva moneda detectada. Conteo total = "));
    Serial.println(coinCounter);

    waitWithServer(1000);
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
      waitWithServer(T_SONIDO);

      //Motor para retirar la moneda
      motor_pulse();
      pumpServer();

    } else {
      // Moneda NO reconocida
      Serial.println(F("[MAIN] Moneda NO reconocida. Solo se activa el motor."));
      motor_pulse();
      pumpServer();
    }
  }

  waitWithServer(10);  // pequeño respiro con atención al servidor
}


// ---------------------------------------------------------------------------------
// Utilitarios de red, estado y EEPROM

/* Function: pumpServer
   Atiende las solicitudes HTTP pendientes del servidor y libera el watchdog.

   Params:
     - Ninguno.

   Returns:
     - void - no retorna valor.

   Restriction:
     Llamar con frecuencia para evitar timeouts en el servidor.
*/
void pumpServer() {
  if (webReady) {
    server.handleClient();
  }
  yield();
}


/* Function: waitWithServer
   Espera un intervalo dado mientras sigue atendiendo el servidor web.

   Params:
     - ms: unsigned long - milisegundos a esperar.

   Returns:
     - void - no retorna valor.

   Restriction:
     Intervalos muy largos deben fraccionarse para no saturar la CPU.
*/
void waitWithServer(unsigned long ms) {
  unsigned long inicio = millis();
  while (millis() - inicio < ms) {
    pumpServer();
    delay(5);
  }
}


/* Function: manageEepromCommit
   Determina si es momento de sincronizar el contador con la EEPROM.

   Params:
     - Ninguno.

   Returns:
     - void - no retorna valor.

   Restriction:
     Debe llamarse periódicamente desde el loop principal.
*/
void manageEepromCommit() {
  if (!eepromDirty) {
    return;
  }

  if (coinsSincePersist >= EEPROM_WRITE_THRESHOLD) {
    persistCounter("conteo");
    return;
  }

  if (millis() - lastCoinMillis >= EEPROM_WRITE_DELAY) {
    persistCounter("timeout");
  }
}


/* Function: persistCounter
   Escribe el contador de monedas en la EEPROM y limpia los indicadores.

   Params:
     - reason: const char* - texto que describe el motivo del guardado.

   Returns:
     - void - no retorna valor.

   Restriction:
     Debe invocarse solo cuando eepromDirty es verdadero.
*/
void persistCounter(const char* reason) {
  uint32_t value = getCoinCounter();
  EEPROM.put(EEPROM_ADDR_COUNTER, value);
  EEPROM.commit();
  lastPersistedCounter = value;
  coinsSincePersist = 0;
  eepromDirty = false;
  Serial.print(F("[EEPROM] Guardado contador ("));
  Serial.print(reason);
  Serial.print(F(") = "));
  Serial.println(value);
}


/* Function: getCoinCounter
   Obtiene el valor actual del contador de monedas en RAM.

   Params:
     - Ninguno.

   Returns:
     - uint32_t - total de monedas acumulado.
*/
uint32_t getCoinCounter() {
  return coinCounter;
}


/* Function: resetCoinCounter
   Restaura el contador de monedas a cero tanto en RAM como en EEPROM.

   Params:
     - Ninguno.

   Returns:
     - void - no retorna valor.

   Restriction:
     Solo debe ejecutarse tras autenticación válida desde la capa web.
*/
void resetCoinCounter() {
  sensorMoneda_setConteo(0);
  coinCounter = 0;
  lastPersistedCounter = 0;
  coinsSincePersist = 0;
  eepromDirty = false;
  EEPROM.put(EEPROM_ADDR_COUNTER, static_cast<uint32_t>(0));
  EEPROM.commit();
  Serial.println(F("[MAIN] Contador reseteado desde web."));
}


/* Function: isCoinSensorActive
   Lee el estado lógico del sensor de moneda para mostrarlo en la interfaz web.

   Params:
     - Ninguno.

   Returns:
     - bool - true si el sensor está activo, de lo contrario false.
*/
bool isCoinSensorActive() {
  return digitalRead(PIN_SENSOR_MONEDA) == HIGH;
}


/* Function: loadCounterFromEEPROM
   Recupera el contador almacenado en EEPROM y prepara los valores iniciales.

   Params:
     - Ninguno.

   Returns:
     - void - no retorna valor.

   Restriction:
     Debe invocarse después de EEPROM.begin antes de usar el contador.
*/
void loadCounterFromEEPROM() {
  uint32_t stored = 0;
  EEPROM.get(EEPROM_ADDR_COUNTER, stored);
  if (stored == 0xFFFFFFFF) {
    stored = 0;
  }
  coinCounter = stored;
  lastPersistedCounter = stored;
  Serial.print(F("[EEPROM] Conteo restaurado = "));
  Serial.println(coinCounter);
}
