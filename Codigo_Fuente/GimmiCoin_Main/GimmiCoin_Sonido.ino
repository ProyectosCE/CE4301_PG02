#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// asume que en GimmiCoin_Main.ino ya definiste:
// #define PIN_DF_RX  D4
// #define PIN_DF_TX  D3
// Si no estuvieran definidos por alguna razón, ponemos valores por defecto.
#ifndef PIN_DF_RX
  #define PIN_DF_RX D4
#endif

#ifndef PIN_DF_TX
  #define PIN_DF_TX D3
#endif

// Pines: RX del ESP (recibe del DFPlayer) y TX del ESP (hacia DFPlayer)
SoftwareSerial dfSerial(PIN_DF_RX, PIN_DF_TX);
DFRobotDFPlayerMini dfPlayer;

static bool sonido_inicializado = false;
static bool sonido_error_fatal  = false;

// Configuración del volumen y pista
const uint8_t SONIDO_VOL_MAXIMO      = 20;   // 0–30
const uint16_t SONIDO_TRACK_MONEDA   = 1;    // 0001.mp3 / 0001.wav en la SD

// Número máximo de intentos de inicialización del DFPlayer
const uint8_t SONIDO_MAX_INTENTOS_INIT = 5;

// Función interna para imprimir detalles del DFPlayer

static void sonido_printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("[SONIDO/DFPlayer] Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("[SONIDO/DFPlayer] Wrong Stack!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("[SONIDO/DFPlayer] Card Inserted."));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("[SONIDO/DFPlayer] Card Removed."));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("[SONIDO/DFPlayer] Card Online."));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("[SONIDO/DFPlayer] Track finished: "));
      Serial.println(value);
      break;
    case DFPlayerError:
      Serial.print(F("[SONIDO/DFPlayer] Error: "));
      switch (value) {
        case Busy:              Serial.println(F("Card not found."));        break;
        case Sleeping:          Serial.println(F("Sleeping."));              break;
        case SerialWrongStack:  Serial.println(F("Wrong Stack."));           break;
        case CheckSumNotMatch:  Serial.println(F("Checksum not match."));    break;
        case FileIndexOut:      Serial.println(F("File index out of range.")); break;
        case FileMismatch:      Serial.println(F("Cannot find file."));      break;
        case Advertise:         Serial.println(F("In Advertise mode."));     break;
        default:                Serial.println(F("Unknown error."));         break;
      }
      break;
    default:
      // Otros eventos no los detallamos
      break;
  }
}

// Inicialización del módulo de sonido

void sonido_init() {
  // Requerimos que Serial ya esté inicializado en el main para ver mensajes.
  Serial.println();
  Serial.println(F("[SONIDO] Inicializando DFPlayer Mini..."));

  // Inicializar el puerto software serial hacia el DFPlayer
  dfSerial.begin(9600);
  delay(500);  // pequeño delay para darle chance al DFPlayer

  bool initOK = false;

  for (uint8_t intento = 1; intento <= SONIDO_MAX_INTENTOS_INIT; intento++) {
    Serial.print(F("[SONIDO] Intento de inicio DFPlayer #"));
    Serial.println(intento);

    if (dfPlayer.begin(dfSerial)) {
      initOK = true;
      break;
    }

    Serial.println(F("[SONIDO] No se pudo iniciar DFPlayer. "
                     "Verifica conexiones, SD card y alimentación."));
    delay(800);  // esperamos un poco antes del siguiente intento
  }

  if (!initOK) {
    Serial.println(F("[SONIDO] ERROR FATAL: DFPlayer no responde tras varios intentos."));
    sonido_inicializado = false;
    sonido_error_fatal  = true;
    return;
  }

  // Configuración básica del DFPlayer
  dfPlayer.setTimeOut(500);                     // Timeout para comunicación
  dfPlayer.volume(SONIDO_VOL_MAXIMO);           // Volumen 0–30
  dfPlayer.EQ(DFPLAYER_EQ_NORMAL);              // Ecualización normal
  dfPlayer.outputDevice(DFPLAYER_DEVICE_SD);    // Usar SD card como fuente

  sonido_inicializado = true;
  sonido_error_fatal  = false;

  Serial.println(F("[SONIDO] DFPlayer inicializado correctamente."));
  Serial.print(F("[SONIDO] Volumen = "));
  Serial.println(SONIDO_VOL_MAXIMO);
}

// Reproducir sonido asociado a la moneda

void sonido_reproMoneda() {
  // Si ya hubo un error fatal, no intentamos nada más
  if (sonido_error_fatal) {
    return;
  }

  // Si no está inicializado (por ejemplo, fallo al inicio del sistema),
  // intentamos una inicialización tardía aquí.
  if (!sonido_inicializado) {
    Serial.println(F("[SONIDO] DFPlayer no estaba inicializado. Intentando inicializar ahora..."));
    sonido_init();
    if (!sonido_inicializado) {
      // Si sigue sin funcionar, no seguimos intentando para no bloquear el sistema
      return;
    }
  }

  // Pedimos reproducir la pista 1 (moneda)
  Serial.println(F("[SONIDO] Reproduciendo pista de moneda..."));
  dfPlayer.play(SONIDO_TRACK_MONEDA);

  // Opcional: revisar si el DFPlayer reporta algo (errores/estado)
  if (dfPlayer.available()) {
    uint8_t type = dfPlayer.readType();
    int value     = dfPlayer.read();
    sonido_printDetail(type, value);
  }
}
