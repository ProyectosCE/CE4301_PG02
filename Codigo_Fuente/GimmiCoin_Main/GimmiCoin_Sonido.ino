// =====================================================================================
// Módulo de sonido (DFPlayer) - Reproducción del efecto de moneda
// =====================================================================================

#if defined(ARDUINO)
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
static SoftwareSerial mySoftwareSerial(PIN_DF_RX, PIN_DF_TX);
static DFRobotDFPlayerMini myDFPlayer;
#else
// Stubs para entorno de desarrollo
struct DFRobotDFPlayerMini {
  bool begin(void*) { return true; }
  void volume(int) {}
  void play(int) {}
};
struct SoftwareSerial {
  SoftwareSerial(int, int) {}
  void begin(int) {}
  void listen() {}
};
static SoftwareSerial mySoftwareSerial(PIN_DF_RX, PIN_DF_TX);
static DFRobotDFPlayerMini myDFPlayer;
#endif

void sonido_init() {
  mySoftwareSerial.begin(9600);
#if defined(ARDUINO)
  Serial.println(F("[SONIDO] Intentando inicializar DFPlayer..."));
  mySoftwareSerial.listen();

  bool dfReady = false;
  for (uint8_t intento = 0; intento < 3 && !dfReady; ++intento) {
    if (intento > 0) {
      Serial.print(F("[SONIDO] Reintento #"));
      Serial.println(static_cast<int>(intento + 1));
      delay(200);
    }

    if (myDFPlayer.begin(mySoftwareSerial)) {
      dfReady = true;
    }
  }

  if (dfReady) {
    myDFPlayer.volume(15);
    Serial.println(F("[SONIDO] DFPlayer inicializado correctamente."));
  } else {
    Serial.println(F("[SONIDO] Error al inicializar DFPlayer tras 3 intentos."));
  }
#else
  myDFPlayer.begin(&mySoftwareSerial);
  Serial.println(F("[SONIDO] DFPlayer simulado inicializado."));
#endif
}

void sonido_reproMoneda() {
  myDFPlayer.play(1);
  Serial.println(F("[SONIDO] Reproduciendo sonido de moneda."));
}