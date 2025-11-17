#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// Pines definidos en el main:
// D3 = RX del ESP8266 (recibe del DFPlayer)
// D4 = TX del ESP8266 (envía al DFPlayer, con resistencia 1k en hardware)

SoftwareSerial dfSerial(D3, D4); 
DFRobotDFPlayerMini dfPlayer;

// Inicialización del módulo de sonido
void sonido_init() {
  dfSerial.begin(9600);
  dfPlayer.begin(dfSerial);
  dfPlayer.volume(28);  // volumen 0 a 30
}

// Reproduce la pista 1 una sola vez
void sonido_reproMoneda() {
  dfPlayer.play(1);   // archivo 0001.wav
}
