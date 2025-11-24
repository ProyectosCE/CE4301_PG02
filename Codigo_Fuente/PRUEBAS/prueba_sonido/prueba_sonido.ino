#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

SoftwareSerial dfSerial(D5, D6);  // RX, TX en D1 mini
DFRobotDFPlayerMini dfPlayer;

void setup() {
  Serial.begin(115200);
  Serial.println(F("Inicializando DFPlayer..."));

  dfSerial.begin(9600);

  if (!dfPlayer.begin(dfSerial)) {
    Serial.println(F("Error iniciando DFPlayer."));
    while (true) { delay(1000); }
  }

  Serial.println(F("DFPlayer listo."));

  dfPlayer.volume(30);   // Volumen 0–30

  dfPlayer.play(1);      // Reproduce 0001.wav (primera pista)
  Serial.println(F("Reproduciendo pista 1..."));
}

void loop() {
  // Si el DFPlayer dejó de reproducir...
  if (dfPlayer.readState() == 0) {
    dfPlayer.play(1);    // ...reproduce de nuevo
    Serial.println(F("Reinicio de reproducción (loop)."));
  }

  delay(200);
}
