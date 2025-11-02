# Arquitectura de Referencia – Gimmighoul Coin Collector

**Proyecto:** Gimmighoul Coin Collector Robot  
**Curso:** CE4301 – Arquitectura de Computadores I  
**Instituto Tecnológico de Costa Rica**  
**Fecha:** Noviembre 2025  

## 1. Resumen del sistema

El sistema es una alcancía inteligente con diseño del Pokémon Gimmighoul. Al insertar una moneda:

1. Un sensor infrarrojo detecta el evento y despierta al sistema si está dormido.
2. El ESP8266 levanta la tapa (servo 1).
3. Pesa la moneda con una celda de carga (HX711).
4. Expulsa/limpia la celda (servo 2).
5. Reproduce un efecto sonoro (DFPlayer + parlante).
6. Actualiza el total y expone los datos por WiFi en modo Punto de Acceso (AP) para que la app móvil los consulte.

## 2. Justificación de decisiones de diseño

- **D1 mini ESP8266 (con WiFi integrado):** reduce tiempo de integración al traer conectividad nativa, bootloader por USB y ecosistema de librerías estable. El equipo ya tiene experiencia previa con este MCU, lo que baja el riesgo y acelera el desarrollo.
- **Clasificación por peso (celda de carga + HX711):** en Costa Rica hay monedas con **diámetros similares** (p. ej., ¢100 y ¢500; ¢25 y ¢50), lo que hace poco fiable la clasificación por “canales” o solo por tiempo de interrupción óptica. El **peso** distingue de forma robusta esos casos y evita errores de rampa/canal.

## 3. Arquitectura de alto nivel (bloques de hardware)

```mermaid
flowchart LR
  subgraph PWR[Alimentación]
    BATT[Batería LiPo 3.7V 2000mAh]
    TP[TP4056<br>carga + protección]
    BOOST[MT3608<br>3.7V → 5V]
    BUCK[LM2596<br>5V → 3.3V]
  end

  subgraph MCU[D1 mini – ESP8266]
    FW[Firmware<br>FSM + AP WiFi + HTTP]
    NVS[Almacenamiento local<br>NVS/Flash]
  end

  IR[Sensor IR<br>detección de moneda]
  LC[Celda de carga 100 g]
  HX[HX711<br>ADC 24 bits]
  S1[Servo SG90<br>tapa]
  S2[Servo SG90<br>limpieza]
  MP3[DFPlayer Mini]
  SPK[Parlante 8Ω/0.5W]

  %% Señales
  IR -->|GPIO digital| MCU
  LC --- HX
  HX -->|DT,SCK| MCU
  MCU -->|PWM| S1
  MCU -->|PWM| S2
  MCU <-->|UART| MP3
  MP3 --- SPK
  MCU <-->|SPIFFS/NVS| NVS

  %% Energía
  BATT --> TP
  TP -->|BAT| BOOST
  BOOST -->|5V| S1
  BOOST -->|5V| S2
  BOOST -->|5V| MP3
  TP -->|BAT| BUCK
  BUCK -->|3.3V| MCU
  BUCK -->|3.3V| HX
```

**Notas eléctricas:**

- Servos y DFPlayer alimentados desde **5 V** (BOOST).
- ESP8266 y HX711 desde **3.3 V** (BUCK).
- **GND común** para todos los módulos.
- Desacoplos: 470–1000 µF en 5 V; 100–220 µF en 3.3 V cerca del ESP8266.

## 4. Arquitectura de software (firmware en ESP8266)

- **Capas:**
  - Drivers: IR (GPIO/interrupt), HX711 (DT/SCK), Servos (PWM), DFPlayer (UART).
  - Servicios: Detección, Pesaje/Filtrado, Limpieza, Audio, Telemetría.
  - Aplicación: **FSM** (máquina de estados) + Exposición HTTP (modo AP).
- **Almacenamiento local:** totales y últimos eventos en **NVS/flash** del ESP8266.

### 4.1 Máquina de estados (FSM)

```mermaid
stateDiagram-v2
  [*] --> IDLE
  IDLE --> DETECT : Trigger IR
  DETECT --> MEASURE : Estabilizar peso (promedio)
  MEASURE --> CLEAN : Valor válido
  CLEAN --> SOUND
  SOUND --> REPORT
  REPORT --> IDLE
  IDLE --> SLEEP : Timeout inactividad
  SLEEP --> IDLE : Wake por IR
```

## 5. Arquitectura de la aplicación móvil

- **Conexión:** la app se conecta a la red **AP** creada por el ESP8266 (sin Internet).
- **Vistas principales:** Total acumulado, Historial básico (desde MCU), Ajustes mínimos.
- **Sin base de datos local:** la app muestra datos consultados al MCU y permite ajustes manuales (ej. retirar monto).

## 6. Interfaces y contratos (AP WiFi + HTTP)

- **Modo AP (Access Point):**
  - El ESP8266 crea una red local (SSID/clave configurables).
  - La app se conecta a ese AP y realiza solicitudes HTTP al **IP del ESP** (por defecto, 192.168.4.1).
  - No requiere router externo.
- **HTTP simple (ejemplos):**
  - `GET /status` → `{ fw:"1.0.0", total_crc: 1234, count: 9 }`
  - `GET /history?limit=50` → `[{ ts:"2025-11-02T20:10Z", peso_g:9.0, valor_crc:100, total_crc:1234 }, ...]`
  - `POST /adjust` → cuerpo `{ delta_crc:-100 }` (para retirar monedas de forma manual desde la app).
- **Periféricos:**
  - DFPlayer: UART 9600 bps (play/pause/volume).
  - HX711: DT/SCK (estilo SPI de 2 hilos).
  - Servos: PWM ~50 Hz.

## 7. Flujos clave (bloques)

### 7.1 Detección → Pesaje → Limpieza → Audio → Reporte

```mermaid
sequenceDiagram
  participant IR as Sensor IR
  participant MCU as ESP8266 (FSM)
  participant HX as HX711
  participant S1 as Servo Tapa
  participant S2 as Servo Limpieza
  participant MP3 as DFPlayer
  participant APP as App Móvil

  IR->>MCU: Pulso (moneda detectada)
  MCU->>S1: Abrir tapa (PWM)
  MCU->>HX: Lecturas múltiples (promedio)
  HX-->>MCU: Peso estable (g)
  MCU->>S2: Expulsar moneda (PWM)
  MCU->>MP3: Reproducir efecto
  MCU->>APP: Exponer total/registro vía HTTP
  APP-->>MCU: Consulta GET /status o /history
```

### 7.2 Gestión de energía

- **Deep Sleep** tras timeout en IDLE; **wake** por interrupción del IR.
- Antes de dormir, se sincroniza el estado mínimo necesario en NVS/flash.

## 9. Seguridad y robustez

- **Eléctrica:** protecciones del TP4056, desacoplos en 5 V/3.3 V, retornos de corriente de servos separados del plano del MCU.
- **Red:** AP con WPA2 y endpoints mínimos; validar inputs en `/adjust` para evitar montos negativos erróneos.
- **Firmware:** validación de rangos (peso, tiempos), reintentos en UART/HX711, watchdog opcional.

## 10. Rendimiento y dimensionamiento

- Latencia objetivo por ciclo completo: ≤ 4 s.
- Precisión de peso: error ≤ ±5 % tras calibración.
- Alcance WiFi interior: 5–8 m en línea de vista.
- Autonomía: según perfil de uso; mediciones documentadas en pruebas de potencia.

## 11. Trazabilidad

- **DoD:** `docs/Definition_of_Done.md`
- **Risk Assessment:** `docs/Risk_Assessment.md`
- **BOM:** `docs/BOM_AlternativaB.md`
- **Roadmap y Issues:** GitHub Projects (Sprints 0–4)

---
