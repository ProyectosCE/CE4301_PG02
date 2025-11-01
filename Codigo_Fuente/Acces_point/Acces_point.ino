#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Nombre y contraseña del Access Point
const char* ssid = "Pajaros_universales";
const char* password = "12345678";

// Crear servidor web en puerto 80
ESP8266WebServer server(80);

// Página de inicio básica
void handleRoot() {
  String html = "<h1>Hola desde ESP8266</h1>";
  html += "<p><a href='/info'>Ver información</a></p>";
  server.send(200, "text/html", html);
}

// Página de información
void handleInfo() {
  String html = "<h2>Información del dispositivo</h2>";
  html += "<p>IP: " + WiFi.softAPIP().toString() + "</p>";
  html += "<p><a href='/'>Volver</a></p>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);

  // Iniciar Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point creado ✅");
  Serial.print("Nombre: "); Serial.println(ssid);
  Serial.print("Contraseña: "); Serial.println(password);
  Serial.print("IP: "); Serial.println(WiFi.softAPIP());

  // Rutas disponibles
  server.on("/", handleRoot);
  server.on("/info", handleInfo);

  // Iniciar servidor
  server.begin();
  Serial.println("Servidor web iniciado ✅");
}

void loop() {
  server.handleClient();
}
