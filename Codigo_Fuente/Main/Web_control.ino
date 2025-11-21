//web control

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


void Start_server(const char* ssid, const char* password){
  WiFi.softAP(ssid, password);
  Serial.println("Access Point creado ");
  Serial.println(WiFi.softAPIP());


  // Rutas
  server.on("/", handleRoot);
  server.on("/estado", handleEstado);

  
  server.begin();
  Serial.println("Servidor web iniciado ");
}