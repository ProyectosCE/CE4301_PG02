//web control

// Aseguramos includes y extern para que el analizador reconozca símbolos
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// Declaraciones externas definidas en Main.ino
extern ESP8266WebServer server;        // servidor HTTP
extern const char* user;               // usuario admin
extern const char* pass;               // password admin
extern volatile unsigned int Counter;  // contador de monedas
extern volatile unsigned int MemwriteLimit; // contador para escritura
extern const int BUTTON_PIN;           // pin del botón
extern const int memDir;               // dirección EEPROM

//apariencia de la web
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Alcancía Inteligente</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
  body { 
    font-family: Arial, sans-serif; 
    text-align: center; 
    background-color: #f0f5f9;
    margin: 0; 
    padding: 40px;
  }
  h1 { color: #0077cc; }
  .card {
    background: white;
    padding: 20px;
    margin: 15px auto;
    border-radius: 10px;
    width: 90%;
    max-width: 350px;
    box-shadow: 0px 4px 10px rgba(0,0,0,0.12);
  }
  .big {
    font-size: 38px;
    font-weight: bold;
    color: #333;
  }
  .state {
    font-size: 26px;
    font-weight: bold;
  }
</style>
</head>
<body>

<h1>Alcancia Inteligente</h1>

<div class="card">
  <p>Total de monedas:</p>
  <p id="pulses" class="big">0</p>
</div>

<div class="card">
  <p>Dinero acumulado:</p>
  <p id="money" class="big">₡0</p>
</div>

<div class="card">
  <p>Estado del boton:</p>
  <p id="pressed" class="state">Cargando...</p>
</div>

<script>
const valorMoneda = 100; // Cambia si tu moneda vale diferente 😀

setInterval(function() {
  fetch('/estado')
    .then(response => response.json())
    .then(data => {
      document.getElementById('pulses').innerText = data.pulses;
      document.getElementById('money').innerText = "$" + (data.pulses * valorMoneda);
      document.getElementById('pressed').innerText =
        data.pressed ? "PRESIONADO" : "LIBRE";
    });
}, 150);
</script>

</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}


//estado de la web


void handleEstado() {
    int state = digitalRead(BUTTON_PIN); //estado del boton

  
  String json = "{\"pulses\":" + String(Counter) +
                ",\"pressed\":" + String(state == HIGH ? "true" : "false") + "}";

  server.send(200, "application/json", json);
}

// Página de administración protegida
void handleAdmin() {
  // Autenticación básica (pide credenciales si no son correctas)
  if (!server.authenticate(user, pass)) {
    return server.requestAuthentication();
  }

  String html = R"rawliteral(
<!DOCTYPE html>
<html><head><title>Admin - Alcancía</title><meta name='viewport' content='width=device-width,initial-scale=1.0'>
<style>
 body{font-family:Arial;margin:30px;background:#f7f9fb;color:#222;}
 h1{color:#005c99;}
 .box{background:#fff;padding:20px;border-radius:12px;max-width:420px;box-shadow:0 4px 10px rgba(0,0,0,.12);}
 button{background:#d62828;color:#fff;border:none;padding:14px 22px;font-size:16px;border-radius:8px;cursor:pointer;}
 button:hover{background:#b71d1d;}
 .value{font-size:30px;font-weight:bold;}
 .ok{color:green;font-weight:bold;}
 .fail{color:red;font-weight:bold;}
</style></head><body>
<h1>Panel Admin</h1>
<div class='box'>
 <p>Monedas actuales:</p>
 <p id='pulses' class='value'>Cargando...</p>
 <p><button onclick='resetCounter()'>Reset contador</button></p>
 <p id='status'></p>
 <p><a href='/'>Volver a principal</a></p>
</div>
<script>
function load(){fetch('/estado').then(r=>r.json()).then(d=>{document.getElementById('pulses').innerText=d.pulses;});}
function resetCounter(){fetch('/reset').then(r=>r.json()).then(d=>{if(d.ok){document.getElementById('status').innerHTML='<span class="ok">Reset correcto</span>';load();}else{document.getElementById('status').innerHTML='<span class="fail">Error</span>';}}).catch(()=>{document.getElementById('status').innerHTML='<span class="fail">Fallo de red</span>';});}
load();
</script>
</body></html>
)rawliteral";

  server.send(200, "text/html", html);
}

// Ruta para resetear el contador (requiere autenticación)
void handleReset() {
  if (!server.authenticate(user, pass)) {
    return server.requestAuthentication();
  }
  // Sección crítica: evitar inconsistencias con la ISR
  noInterrupts();
  Counter = 0;
  MemwriteLimit = 0;
  interrupts();
  EEPROM.write(memDir, 0);
  EEPROM.commit();
  server.send(200, "application/json", "{\"ok\":true,\"pulses\":0}");
}


void Start_server(const char* ssid, const char* password){
  WiFi.softAP(ssid, password);
  Serial.println("Access Point creado ");
  Serial.println(WiFi.softAPIP());


  // Rutas
  server.on("/", handleRoot);
  server.on("/estado", handleEstado);
  server.on("/admin", handleAdmin);
  server.on("/reset", handleReset);

  
  server.begin();
  Serial.println("Servidor web iniciado ");
}