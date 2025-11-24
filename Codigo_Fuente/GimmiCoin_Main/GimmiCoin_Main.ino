#if defined(__INTELLISENSE__)
#undef ARDUINO
#endif

#if defined(ARDUINO)
#include <Arduino.h>
#else
#include "arduino_stub.h"
#endif

#if defined(ARDUINO)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#else
#include "esp8266_stubs.h"
#endif

#if defined(ARDUINO)
#include <EEPROM.h>
#else
#include "eeprom_stub.h"
#endif

#define PIN_SENSOR_MONEDA   D2     // Sensor de moneda (pull-down externo)
#define PIN_SERVO           D1     // Servo SG92R
#define PIN_MOTOR           D8     // Motor DC (transistor)

#define PIN_DF_RX           D4     // DFPlayer: RX del ESP
#define PIN_DF_TX           D3     // DFPlayer: TX del ESP (con resistencia 1k)

// Pines de la balanza (HX711)
#define PIN_BALANZA_DOUT    D5     // HX711 DT
#define PIN_BALANZA_SCK     D6     // HX711 SCK

// Duraciones principales
const int T_SONIDO     = 2000;
const unsigned long PRE_MEASURE_DELAY_MS = 1000;
const unsigned long SERVO_OPEN_SETTLE_MS = 300;
const unsigned long SERVO_CLOSE_SETTLE_MS = 2000;
constexpr unsigned long DEEP_SLEEP_SETTLE_MS = 250;  // Tiempo para que la web refleje el último estado

// Valores monetarios por tipo de moneda identificado
constexpr uint32_t COIN_VALUE_10   = 10;
constexpr uint32_t COIN_VALUE_25   = 25;
constexpr uint32_t COIN_VALUE_50   = 50;
constexpr uint32_t COIN_VALUE_100  = 100;
constexpr uint32_t COIN_VALUE_500  = 500;

// Configuración de red y autenticación web
const char* WIFI_AP_SSID     = "GimmiCoin";
const char* WIFI_AP_PASSWORD = "12345678";
const char* WEB_ADMIN_USER   = "admin";
const char* WEB_ADMIN_PASS   = "1234";

ESP8266WebServer server(80);

// Persistencia del contador en EEPROM
constexpr uint16_t EEPROM_BYTES            = 16;
constexpr int      EEPROM_ADDR_COUNTER     = 0;
constexpr int      EEPROM_ADDR_MONEY       = EEPROM_ADDR_COUNTER + sizeof(uint32_t);
constexpr uint8_t  EEPROM_WRITE_THRESHOLD  = 10;       // Guardar cada 10 monedas
constexpr unsigned long EEPROM_WRITE_DELAY = 15000UL;  // O tras 15 s sin nuevas monedas

static uint32_t coinCounter = 0;           // Conteo total persistente
static uint32_t lastPersistedCounter = 0;  // Último valor guardado en EEPROM
static uint32_t coinsSincePersist = 0;     // Conteo desde el último guardado
static bool     eepromDirty = false;       // Indica si hay datos pendientes
static unsigned long lastCoinMillis = 0;   // Última moneda detectada
static uint32_t moneyTotal = 0;            // Monto total acumulado en colones
static uint32_t lastPersistedMoney = 0;    // Último monto guardado en EEPROM
static bool pendingDeepSleep = false;      // Señala si debe entrar en deep sleep tras sincronizar
static unsigned long deepSleepScheduledAt = 0;
static bool deepSleepEngaged = false;      // Marca que ya se ejecutó la ruta de deep sleep
bool webReady = false;

// FSM principal que gobierna el flujo de la alcancía
enum class SystemState : uint8_t {
  Idle,
  MeasureCoin,
  ActuateRecognized,
  ActuateUnrecognized,
  Finalize
};

struct StateContext {
  SystemState state = SystemState::Idle;
  bool measurementDone = false;
  bool measurementStable = false;
  bool actuationDone = false;
  float lastWeight = 0.0F;
  int lastCoinValue = 0;
  unsigned long stateSince = 0;
};

static StateContext fsmCtx;

// Declaraciones adelantadas para utilitarios web/persistencia
void startWebServer(const char* ssid, const char* password);
void pumpServer();
void waitWithServer(unsigned long ms);
void manageEepromCommit();
void persistCounter(const char* reason, bool dueToInactivity);
uint32_t getCoinCounter();
uint32_t getMoneyTotal();
void resetCoinCounter();
bool isCoinSensorActive();
void loadCounterFromEEPROM();
const char* stateToString(SystemState state);
void transitionTo(SystemState nextState);
void processStateMachine();
void scheduleDeepSleep();
void maybeEnterDeepSleep();
void enterDeepSleepNow();

// Declaraciones adelantadas de módulos hardware
void sensorMoneda_setConteo(uint32_t value);

// Sensor de moneda
void sensorMoneda_init();
bool sensorMoneda_hayNuevaMoneda();
uint32_t sensorMoneda_getConteo();

// Motor 
void motor_init();
void motor_pulse();

// Servo
void servo_init();
void servo_irAAbierto();
void servo_irACerrado();

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
  servo_init();
  motor_init();
  sonido_init();
  balanza_init(PIN_BALANZA_DOUT, PIN_BALANZA_SCK);

  startWebServer(WIFI_AP_SSID, WIFI_AP_PASSWORD);

  transitionTo(SystemState::Idle);

  pendingDeepSleep = false;
  deepSleepScheduledAt = 0;
  deepSleepEngaged = false;

  lastCoinMillis = millis();

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
  processStateMachine();
  manageEepromCommit();
  maybeEnterDeepSleep();
  waitWithServer(5);
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


/* Function: stateToString
   Devuelve el nombre simbólico de un estado de la FSM para fines de depuración.

   Params:
     - state: SystemState - estado actual o destino que se desea describir.

   Returns:
     - const char* - cadena literal con la etiqueta del estado.
*/
const char* stateToString(SystemState state) {
  switch (state) {
    case SystemState::Idle:                return "IDLE";
    case SystemState::MeasureCoin:         return "MEASURE";
    case SystemState::ActuateRecognized:   return "ACTUATE_RECOGNIZED";
    case SystemState::ActuateUnrecognized: return "ACTUATE_UNRECOGNIZED";
    case SystemState::Finalize:            return "FINALIZE";
    default:                               return "UNKNOWN";
  }
}


/* Function: transitionTo
   Cambia el estado actual de la FSM y registra la marca de tiempo del cambio.

   Params:
     - nextState: SystemState - estado objetivo al que se desea transicionar.

   Returns:
     - void - no retorna valor.
*/
void transitionTo(SystemState nextState) {
  fsmCtx.state = nextState;
  fsmCtx.stateSince = millis();
  Serial.print(F("[FSM] -> "));
  Serial.println(stateToString(nextState));
}


/* Function: processStateMachine
   Ejecuta un ciclo de la máquina de estados finita que modela la alcancía.

   Params:
     - Ninguno.

   Returns:
     - void - no retorna valor.

   Restriction:
     Debe llamarse con alta frecuencia para mantener la respuesta del sistema.
*/
void processStateMachine() {
  switch (fsmCtx.state) {
    case SystemState::Idle:
      if (sensorMoneda_hayNuevaMoneda()) {
        lastCoinMillis = millis();
        pendingDeepSleep = false;
        deepSleepScheduledAt = 0;
        deepSleepEngaged = false;

        Serial.println();
        Serial.print(F("[FSM] Moneda detectada. Conteo actual = "));
        Serial.println(coinCounter);

        fsmCtx.lastWeight = 0.0F;
  fsmCtx.lastCoinValue = 0;
        fsmCtx.measurementDone = false;
        fsmCtx.measurementStable = false;
        fsmCtx.actuationDone = false;

        transitionTo(SystemState::MeasureCoin);
      }
      break;

    case SystemState::MeasureCoin:
      if (millis() - fsmCtx.stateSince < PRE_MEASURE_DELAY_MS) {
        return;
      }

      if (!fsmCtx.measurementDone) {
        Serial.println(F("[FSM] Iniciando medición de moneda."));
        fsmCtx.measurementStable = balanza_medirMonedaEstable(fsmCtx.lastWeight, fsmCtx.lastCoinValue);
        fsmCtx.measurementDone = true;

        Serial.print(F("[FSM] Peso medido = "));
        Serial.print(fsmCtx.lastWeight, 2);
        Serial.println(F(" g"));

        if (fsmCtx.lastCoinValue > 0) {
          Serial.print(F("[FSM] Valor detectado = ₡"));
          Serial.println(fsmCtx.lastCoinValue);
        } else {
          Serial.println(F("[FSM] Valor detectado inválido."));
        }

        if (!fsmCtx.measurementStable) {
          Serial.println(F("[FSM] Aviso: peso no totalmente estable (timeout)."));
        }
      }

      if (fsmCtx.lastCoinValue > 0) {
        transitionTo(SystemState::ActuateRecognized);
      } else {
        transitionTo(SystemState::ActuateUnrecognized);
      }
      break;

    case SystemState::ActuateRecognized:
      if (!fsmCtx.actuationDone) {
        Serial.print(F("[FSM] Actuando moneda reconocida (₡"));
        Serial.print(fsmCtx.lastCoinValue);
        Serial.println(F(")"));

        const uint32_t coinValue = static_cast<uint32_t>(fsmCtx.lastCoinValue);
        coinCounter += 1;
        coinsSincePersist = coinCounter - lastPersistedCounter;
        moneyTotal += coinValue;
        eepromDirty = true;
        lastCoinMillis = millis();

        Serial.print(F("[FSM] Conteo reconocido = "));
        Serial.println(coinCounter);
        Serial.print(F("[FSM] Valor acreditado = ₡"));
        Serial.println(coinValue);
        Serial.print(F("[FSM] Total acumulado = ₡"));
        Serial.println(moneyTotal);

        switch (coinValue) {
          case COIN_VALUE_10:
            Serial.println(F("[FSM] Denominación: 10 colones"));
            break;
          case COIN_VALUE_25:
            Serial.println(F("[FSM] Denominación: 25 colones"));
            break;
          case COIN_VALUE_50:
            Serial.println(F("[FSM] Denominación: 50 colones"));
            break;
          case COIN_VALUE_100:
            Serial.println(F("[FSM] Denominación: 100 colones"));
            break;
          case COIN_VALUE_500:
            Serial.println(F("[FSM] Denominación: 500 colones"));
            break;
          default:
            Serial.println(F("[FSM] Denominación: desconocida (valor reconocido)."));
            break;
        }

        sonido_reproMoneda();
        waitWithServer(T_SONIDO);
        servo_irAAbierto();
        waitWithServer(SERVO_OPEN_SETTLE_MS);
        servo_irACerrado();
        waitWithServer(SERVO_CLOSE_SETTLE_MS);
        motor_pulse();

        fsmCtx.actuationDone = true;
      }

      transitionTo(SystemState::Finalize);
      break;

    case SystemState::ActuateUnrecognized:
      if (!fsmCtx.actuationDone) {
        Serial.println(F("[FSM] Peso inválido o moneda desconocida. Omitiendo accionamiento."));
        fsmCtx.actuationDone = true;
      }

      transitionTo(SystemState::Finalize);
      break;

    case SystemState::Finalize:
      Serial.println(F("[FSM] Ciclo de moneda completado."));
      fsmCtx.measurementDone = false;
      fsmCtx.measurementStable = false;
      fsmCtx.actuationDone = false;
      fsmCtx.lastCoinValue = 0;
      fsmCtx.lastWeight = 0.0F;
      sensorMoneda_setConteo(coinCounter);
      transitionTo(SystemState::Idle);
      break;
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
  const unsigned long inactivity = millis() - lastCoinMillis;

  if (deepSleepEngaged) {
    return;
  }

  if (eepromDirty) {
    if (coinsSincePersist >= EEPROM_WRITE_THRESHOLD) {
      persistCounter("conteo", false);
      return;
    }

    if (inactivity >= EEPROM_WRITE_DELAY) {
      persistCounter("timeout", true);
    }
    return;
  }

  if (!pendingDeepSleep && inactivity >= EEPROM_WRITE_DELAY) {
    Serial.println(F("[MAIN] Inactividad prolongada sin cambios. Programando deep sleep."));
    scheduleDeepSleep();
  }
}


/* Function: persistCounter
   Escribe el contador de monedas en la EEPROM y limpia los indicadores. Puede
   opcionalmente programar la entrada a deep sleep cuando la sincronización se
   produjo por inactividad.

   Params:
     - reason: const char* - texto que describe el motivo del guardado.
     - dueToInactivity: bool - true si se invoca por timeout de inactividad.

   Returns:
     - void - no retorna valor.

   Restriction:
     Debe invocarse solo cuando eepromDirty es verdadero.
*/
void persistCounter(const char* reason, bool dueToInactivity) {
  const uint32_t counterValue = getCoinCounter();
  const uint32_t moneyValue   = getMoneyTotal();

  EEPROM.put(EEPROM_ADDR_COUNTER, counterValue);
  EEPROM.put(EEPROM_ADDR_MONEY, moneyValue);
  EEPROM.commit();

  lastPersistedCounter = counterValue;
  lastPersistedMoney   = moneyValue;
  coinsSincePersist    = 0;
  eepromDirty          = false;

  Serial.print(F("[EEPROM] Guardado contador ("));
  Serial.print(reason);
  Serial.print(F(") = "));
  Serial.println(counterValue);
  Serial.print(F("[EEPROM] Dinero acumulado = ₡"));
  Serial.println(moneyValue);

  if (dueToInactivity) {
    scheduleDeepSleep();
  }
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

uint32_t getMoneyTotal() {
  return moneyTotal;
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
  moneyTotal = 0;
  lastPersistedMoney = 0;
  eepromDirty = false;
  pendingDeepSleep = false;
  deepSleepScheduledAt = 0;
  EEPROM.put(EEPROM_ADDR_COUNTER, static_cast<uint32_t>(0));
  EEPROM.put(EEPROM_ADDR_MONEY, static_cast<uint32_t>(0));
  EEPROM.commit();
  lastCoinMillis = millis();
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
  coinsSincePersist = 0;

  uint32_t storedMoney = 0;
  EEPROM.get(EEPROM_ADDR_MONEY, storedMoney);
  if (storedMoney == 0xFFFFFFFF) {
    storedMoney = 0;
  }
  moneyTotal = storedMoney;
  lastPersistedMoney = storedMoney;

  Serial.print(F("[EEPROM] Dinero restaurado = ₡"));
  Serial.println(moneyTotal);
}


void scheduleDeepSleep() {
  if (deepSleepEngaged || pendingDeepSleep) {
    return;
  }
  pendingDeepSleep = true;
  deepSleepScheduledAt = millis();
  Serial.println(F("[POWER] Deep sleep programado tras inactividad."));
}


void maybeEnterDeepSleep() {
  if (deepSleepEngaged) {
    return;
  }

  if (!pendingDeepSleep) {
    return;
  }

  pumpServer();

  if (millis() - deepSleepScheduledAt < DEEP_SLEEP_SETTLE_MS) {
    return;
  }

  enterDeepSleepNow();
}


void enterDeepSleepNow() {
  if (deepSleepEngaged) {
    return;
  }

  Serial.println(F("[POWER] Entrando en deep sleep. Esperando nueva interrupción."));
  pendingDeepSleep = false;
  deepSleepScheduledAt = 0;
  deepSleepEngaged = true;

#if GM_USING_ARDUINO_CORE
#if defined(ARDUINO_ARCH_ESP8266)
  WiFi.forceSleepBegin();
  delay(1);
  ESP.deepSleep(0);
#else
  delay(50);
  ESP.deepSleep(0);
#endif
#else
  Serial.println(F("[POWER] Deep sleep simulado (entorno de desarrollo)."));
#endif
}
