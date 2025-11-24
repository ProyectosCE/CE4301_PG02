<div align="center">

# GimmiCoin – Alcancía Inteligente (Proyecto CE4301)

“Gimmighoul” convertido en una alcancía IoT que detecta, pesa y registra monedas, reproduce audio y expone su estado por una interfaz web con panel de administración.”

</div>

---

## 1. Propósito del Proyecto
Este proyecto implementa una alcancía inteligente basada en un Mini D1 Wemos ESP8266. El sistema detecta la inserción de una moneda, lee su peso mediante una celda de carga + HX711 para clasificación, acciona un motor DC para desplazamiento, reproduce un sonido con un módulo DFPlayer Mini y mantiene un conteo persistente en EEPROM. Además publica un servidor web local (modo Access Point) con:
- Página pública con total de monedas y estado del sensor.
- Panel `/admin` protegido (Basic Auth) para reset del contador.

> Referencia conceptual y narrativa: personaje Pokémon “Gimmighoul” (cofre + monedas). El objetivo académico incluye integración de sensores, actuadores, persistencia y comunicación embebida.

---

## 2. Características Clave
| Área | Característica |
|------|---------------|
| Detección | Sensor (entrada digital con pull-down) e y detección de conductividad mediante puente de conductores. |
| Pesaje | Celda de carga TAL221 + HX711 (24-bit ADC). |
| Movimiento | Motor DC para mover la moneda al interior. |
| Audio | DFPlayer Mini + parlante 8Ω 0.5 W (efecto al registrar moneda). |
| Persistencia | Conteo guardado en EEPROM. El conteo se guarda despues de un timeout de 30 segundos despúes de que se detectó la última moneda. |
| Conectividad | Access Point propio (`ssid` configurable) + servidor HTTP embebido. |
| Interfaz Web | `/` estado general; `/estado` JSON; `/admin` + `/reset` (auth). |
| Administración | Reset seguro del contador (bloqueo de interrupciones). |
| Escalabilidad | Posible futura clasificación por rangos de peso y envío remoto. |

---

## 3. Flujo de Operación
1. Moneda atraviesa sensor → flanco detectado en pin de interrupción.
2. Se valida flanco y antirrebote (intervalo mínimo definido) → incrementa `buffer Counter`.
3. Lectura de peso y clasificación por gramos → tipo de moneda.
4. Motor DC mueve la moneda al interior.
5. Módulo DFPlayer reproduce sonido.
6. Contador se guarda en EEPROM después de un timeout de 30 segundos tras la última moneda detectada.
7. Cliente web consulta `/estado` para actualizar interfaz (polling). Reset opcional en `/admin`.

---

## 4. Arquitectura (Resumen)
| Capa | Componentes |
|------|-------------|
| Hardware | ESP8266 D1 Mini, HX711, Celda de carga 100 g, DFPlayer, Servos SG90, Sensor IR / paleta mecánica, Reguladores energía |
| Firmware | Módulos `.ino`: `Main`, `Interrupt`, `Web_control`, (balanza, sonido, servo, motor, detección). |
| Persistencia | EEPROM dirección `0` para `Counter`. |
| Comunicación | HTTP sobre modo SoftAP (sin backend externo). |
| Interfaz | HTML + JS embebido (polling cada 150 ms). |

Diagrama lógico (pendiente de agregar):
```
Sensor -> ISR -> (Debounce) -> Counter -> (cada 10) EEPROM
                   |                          |
                   +--> Servo + Audio         +--> Web /estado
```

---

## 5. Estructura del Repositorio (Extracto)
```
Codigo_Fuente/
  Main/
    Main.ino              # Setup principal + loop + persistencia
    Interrupt.ino         # ISR de conteo
    Web_control.ino       # Rutas HTTP, admin y reset
  GimmiCoin_Main/         # Módulos de sonido, balanza, servo, motor
Documentacion/
  main.tex                # Anteproyecto (Alternativas y justificación)
  BOM.md                  # Lista de materiales detallada
  readme.md               # Este archivo
Hardware/                 # Diagramas y modelos Fritzing
```

---

## 6. Lista de Materiales (Resumen BOM)
Tomado de `BOM.md` (ver archivo para detalle completo y costos en USD / colones).

| Categoría | Componentes |
|-----------|-------------|
| Microcontrolador | Mini D1 Wemos ESP8266 |
| Sensores | Celda de carga TAL221 (100 g), HX711, Sensor casero de conductividad |
| Actuadores | Motor DC |
| Audio | DFPlayer Mini, Parlante 8Ω 0.5 W |
| Alimentación | Powebank 10000 mAh output:5V 3 A|
| Conexión | Cables, headers, tornillería ligera |
| Otros | Material reciclado (cartón, plastico y madera) para la estructura |

Total estimado (sin impresión 3D): ≈ $99

---

## 7. Dependencias de Firmware
Instalar (Arduino IDE / PlatformIO):
- Núcleo ESP8266 (Board Manager) – D1 Mini.
- Librerías:
  - `ESP8266WiFi.h`
  - `ESP8266WebServer.h`
  - `EEPROM.h`
  - `SoftwareSerial.h` (para DFPlayer si no se usa puerto hardware)
  - `DFRobotDFPlayerMini.h` (módulo MP3)
  - `Servo.h` (control de SG90) – si aplica en módulos servo.
  - `HX711.h` (lectura de balanza) – pendiente confirmar versión usada.
  - `Arduino.h`

> Nota: Ajustar pines según colisiones (SoftwareSerial en ESP8266 tiene limitaciones). Mantener volumen DFPlayer en rango 0–30.

---

## 8. Endpoints HTTP
| Ruta | Método | Auth | Descripción |
|------|--------|------|-------------|
| `/` | GET | No | Página principal (HTML) con conteo y estado. |
| `/estado` | GET | No | JSON: `{ pulses, pressed }` |
| `/admin` | GET | Sí (Basic) | Panel admin con botón reset. |
| `/reset` | GET | Sí (Basic) | Reinicia contador y EEPROM. |

Ejemplo respuesta `/estado`:
```json
{ "pulses": 24, "pressed": true }
```

Autenticación: `user="admin"`, `pass="1234"` (definidos en `Main.ino`). **Se recomienda cambiar en producción.**

---

## 9. Persistencia y Ciclo EEPROM
El contador incrementa después de 30 segundos sin nuevas monedas detectadas (timeout). Cada vez que se detecta una moneda, se reinicia el timeout. Esto reduce el número de escrituras en EEPROM, dismuniyendo el acceso a memoria y evitando gastar ciclos de escritura innecesarios.

---

## 10. Interrupciones y Antirrebote
Contador de monedas por conductividad:
Utiliza un debounce basado en tiempo donde se valida el flanco y se asegura un intervalo mínimo entre pulsos.
- Flanco válido: HIGH → LOW.
- Intervalo mínimo `minPulseInterval` (actual: 300,000 µs ≈ 0.3 s) para evitar doble conteo. Se sugiere revisar antes de pruebas funcionales con múltiples monedas.

---

## 11. Calibración de Peso
Pasos futuros:
1. Rutina de tare de la celda, se realiza al leer offset base sin carga.
2. Lecturas promedio para estabilizar ruido.
3. Tabla de rangos, donde los gramos → tipo de moneda, esto basado en información tomada del sitio oficial del BCCR.

---

## 12. Compilación y Ejecución (Pendiente)
Se documentará:
- Selección de placa y versión núcleo ESP8266.
- Parámetros de velocidad serie y baudios (115200).
- Orden recomendado de conexión hardware.
- Ajustes de pines para DFPlayer / HX711 / Servos.

Check-list futuro:
```
[ ] Instalar dependencias
[ ] Configurar SSID / password
[ ] Ajustar minPulseInterval
[ ] Calibrar balanza
[ ] Verificar endpoints web
[ ] Pruebas con varias monedas
```

---

## 13. Módulos y Archivos Principales
| Archivo | Rol |
|---------|-----|
| `Main/ Main.ino` | Setup, loop, persistencia EEPROM, arranque AP. |
| `Main/ Interrupt.ino` | ISR de conteo de monedas. |
| `Main/ Web_control.ino` | Rutas HTTP, panel admin, reset. |
| `GimmiCoin_Main/ GimmiCoin_Sonido.ino` | Inicialización DFPlayer y reproducción. |
| `GimmiCoin_Main/ GimmiCoin_Servo.ino` | Movimientos de servos (pendiente ver lógica final). |
| `GimmiCoin_Main/ GimmiCoin_Balanza.ino` | Lectura HX711 (calibración futura). |
| `Documentacion/ main.tex` | Anteproyecto completo (Alternativas A y B). |
| `Documentacion/ BOM.md` | Lista de materiales detallada y costos. |

---
