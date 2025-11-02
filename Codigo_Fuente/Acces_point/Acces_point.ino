#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

const char* ssid = "Pajaros_universales";
const char* password = "12345678";

const int BUTTON_PIN = 5; // GPIO5 = D1
volatile unsigned int veces = 0;
unsigned int Guardarcada10 = 0;
volatile bool buttonPressed = false;

volatile unsigned long lastInterruptTime = 0;

ESP8266WebServer server(80);

const int memDir = 0;

// Página WEB con AJAX
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Estado del Botón</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
  body { font-family: Arial; text-align: center; margin-top: 70px; }
  #estado { font-size: 30px; font-weight: bold; }
</style>
</head>
<body>

<h1>ESP8266 - Botón</h1>
<p>Veces presionado:</p>
<p id="pulses">0</p>

<p>Estado actual:</p>
<p id="pressed">Cargando...</p>

<script>
setInterval(function() {
  fetch('/estado')
    .then(response => response.json())
    .then(data => {
      document.getElementById('pulses').innerText = data.pulses;
      document.getElementById('pressed').innerText = data.pressed ? "PRESIONADO ✅" : "NO presionado ❌";
    });
}, 150);
</script>

</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}


void IRAM_ATTR Contar_Pulsos() {
  unsigned long currentTime = micros();

  if (currentTime - lastInterruptTime > 120000) {  
    veces++;
    Guardarcada10++;
  }

  lastInterruptTime = currentTime;
}

// Nueva ruta: solo devuelve texto
void handleEstado() {
    int state = digitalRead(BUTTON_PIN);

  
  String json = "{\"pulses\":" + String(veces) +
                ",\"pressed\":" + String(state == LOW ? "true" : "false") + "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  Serial.println("Access Point creado ✅");
  Serial.println(WiFi.softAPIP());

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Rutas
  server.on("/", handleRoot);
  server.on("/estado", handleEstado);

  server.begin();
  Serial.println("Servidor web iniciado ✅");

  EEPROM.begin(10); //reservamos 10 bytes, 
  veces = EEPROM.read(memDir);
  Serial.print("Valor recuperado: ");
  Serial.println(veces);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), Contar_Pulsos, FALLING);




}

void loop() {
  server.handleClient();
   if (Guardarcada10 >= 10) {
    EEPROM.write(memDir, veces);
    EEPROM.commit();
    Guardarcada10 = 0;
    Serial.println("Guardado en EEPROM ✅");
  }



}
