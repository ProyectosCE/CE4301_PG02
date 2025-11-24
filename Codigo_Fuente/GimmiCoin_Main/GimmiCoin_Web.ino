#if !defined(__XTENSA__)
#undef ARDUINO
#undef GM_USING_ARDUINO_CORE
#define GM_USING_ARDUINO_CORE 0
#endif
#include "arduino_compat.h"

#include <stdio.h>

#if GM_USING_ARDUINO_CORE
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#else
#include "esp8266_stubs.h"
#endif

extern const char* WEB_ADMIN_USER;
extern const char* WEB_ADMIN_PASS;
extern ESP8266WebServer server;
extern bool webReady;

uint32_t getCoinCounter();
uint32_t getMoneyTotal();
bool isCoinSensorActive();
void resetCoinCounter();

static void handleRoot();
static void handleEstado();
static void handleAdmin();
static void handleReset();

/* Function: startWebServer
	 Configura el modo Access Point y registra las rutas HTTP expuestas.

	 Params:
		 - ssid: const char* - nombre de la red Wi-Fi a publicar.
		 - password: const char* - contraseña del Access Point.

	 Returns:
		 - void - no retorna valor.

	 Restriction:
		 Debe llamarse una vez finalizada la configuración de hardware.
*/
void startWebServer(const char* ssid, const char* password) {
  WiFi.softAP(ssid, password);
  Serial.println(F("[WEB] Access Point creado"));
  Serial.print(F("[WEB] IP AP: "));
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/estado", handleEstado);
  server.on("/admin", handleAdmin);
  server.on("/reset", handleReset);

  server.begin();
  webReady = true;
  Serial.println(F("[WEB] Servidor HTTP iniciado"));
}

/* Function: handleRoot
	 Entrega la página principal con el estado resumido del sistema.

	 Params:
		 - Ninguno.

	 Returns:
		 - void - no retorna valor.
*/
static void handleRoot() {
	const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Alcancía Inteligente</title>
	<meta charset="UTF-8">
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
		max-width: 360px;
		box-shadow: 0 4px 10px rgba(0,0,0,0.12);
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

<h1>GimmiCoin</h1>

<div class="card">
	<p>Total de monedas:</p>
	<p id="pulses" class="big">0</p>
</div>

<div class="card">
	<p>Dinero acumulado:</p>
	<p id="money" class="big">₡0</p>
</div>

<div class="card">
	<p>Estado del sensor:</p>
	<p id="pressed" class="state">Cargando...</p>
</div>

<script>
setInterval(function() {
	fetch('/estado')
		.then(response => response.json())
		.then(data => {
			document.getElementById('pulses').innerText = data.pulses;
			document.getElementById('money').innerText = '₡' + data.money;
			document.getElementById('pressed').innerText = data.pressed ? 'DETECTADO' : 'LIBRE';
		})
		.catch(() => {
			document.getElementById('pressed').innerText = 'SIN CONEXIÓN';
		});
}, 500);
</script>

</body>
</html>
)rawliteral";

	server.send(200, "text/html; charset=utf-8", html);
}

/* Function: handleEstado
	 Responde con el estado en formato JSON para consultas AJAX.

	 Params:
		 - Ninguno.

	 Returns:
		 - void - no retorna valor.
*/
static void handleEstado() {
  const bool activo = isCoinSensorActive();
	char buffer[96];
	snprintf(buffer,
					 sizeof(buffer),
					 "{\"pulses\":%lu,\"money\":%lu,\"pressed\":%s}",
					 static_cast<unsigned long>(getCoinCounter()),
					 static_cast<unsigned long>(getMoneyTotal()),
					 activo ? "true" : "false");
		server.send(200, "application/json", buffer);
}

/* Function: handleAdmin
	 Sirve el panel de administración protegido con autenticación básica.

	 Params:
		 - Ninguno.

	 Returns:
		 - void - no retorna valor.

	 Restriction:
		 Requiere credenciales válidas definidas en el firmware.
*/
static void handleAdmin() {

  if (!server.authenticate(WEB_ADMIN_USER, WEB_ADMIN_PASS)) {
    return server.requestAuthentication();
  }

	const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Admin GimmiCoin</title>
	<meta charset="UTF-8">
<meta name='viewport' content='width=device-width, initial-scale=1.0'>
<style>
 body{font-family:Arial;margin:30px;background:#f7f9fb;color:#222;}
 h1{color:#005c99;}
 .box{background:#fff;padding:20px;border-radius:12px;max-width:420px;box-shadow:0 4px 10px rgba(0,0,0,.12);}
 button{background:#d62828;color:#fff;border:none;padding:14px 22px;font-size:16px;border-radius:8px;cursor:pointer;}
 button:hover{background:#b71d1d;}
 .value{font-size:30px;font-weight:bold;}
 .ok{color:green;font-weight:bold;}
 .fail{color:red;font-weight:bold;}
</style>
</head>
<body>
<h1>Panel Admin</h1>
<div class='box'>
 <p>Monedas actuales:</p>
 <p id='pulses' class='value'>Cargando...</p>
 <p>Dinero acumulado:</p>
 <p id='money' class='value'>Cargando...</p>
 <p><button onclick='resetCounter()'>Reset contador</button></p>
 <p id='status'></p>
 <p><a href='/'>Volver a principal</a></p>
</div>
<script>
function load(){fetch('/estado').then(r=>r.json()).then(d=>{document.getElementById('pulses').innerText=d.pulses;document.getElementById('money').innerText='₡'+d.money;});}
function resetCounter(){fetch('/reset').then(r=>r.json()).then(d=>{if(d.ok){document.getElementById('status').innerHTML='<span class="ok">Reset correcto</span>';document.getElementById('pulses').innerText=d.pulses;document.getElementById('money').innerText='₡'+d.money;}else{document.getElementById('status').innerHTML='<span class="fail">Error</span>';}}).catch(()=>{document.getElementById('status').innerHTML='<span class="fail">Fallo de red</span>';});}
load();
</script>
</body>
</html>
)rawliteral";

	server.send(200, "text/html; charset=utf-8", html);
}

/* Function: handleReset
	 Ejecuta el reseteo remoto del contador tras validar credenciales.

	 Params:
		 - Ninguno.

	 Returns:
		 - void - no retorna valor.

	 Restriction:
		 Solo debe invocarse desde el panel protegido.
*/
static void handleReset() {
  if (!server.authenticate(WEB_ADMIN_USER, WEB_ADMIN_PASS)) {
    return server.requestAuthentication();
  }

  resetCoinCounter();

	char buffer[96];
	snprintf(buffer,
					sizeof(buffer),
					"{\"ok\":true,\"pulses\":%lu,\"money\":%lu}",
					static_cast<unsigned long>(getCoinCounter()),
					static_cast<unsigned long>(getMoneyTotal()));
			server.send(200, "application/json", buffer);
}
