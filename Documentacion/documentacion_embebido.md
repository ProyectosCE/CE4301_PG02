# Arquitectura del Software Embebido – GimmiCoin

## 1. Visión General

El firmware de **GimmiCoin** se ejecuta sobre un **ESP8266 (D1 Mini)** y controla todo el comportamiento de la alcancía inteligente:

- Detección de monedas mediante un **sensor de contacto metálico**.
- Lectura y filtrado de peso con **celda de carga TAL221 + HX711**.
- Accionamiento de un **motor DC** para limpiar la plataforma de pesaje.
- Reproducción de audio con **DFPlayer Mini**.
- Exposición de un **servidor web HTTP** en modo *Access Point*.
- Persistencia del total acumulado en **EEPROM/Flash**.
- Transición a **deep sleep** para ahorrar energía tras un período de inactividad.

El diseño se basa en una **máquina de estados finita (FSM)** que coordina la lógica de alto nivel, apoyada por módulos especializados (.ino) para cada subsistema.

---

## 2. Organización del Firmware

El código fuente se organiza en módulos, cada uno con responsabilidades claras:

- **`GimmiCoin_Main.ino`**
  - `setup()` y `loop()`.
  - Inicialización global de pines, WiFi AP y servidor HTTP.
  - Orquestación de la FSM y llamados a los demás módulos.
- **`GimmiCoin_SensorMoneda.ino`**
  - Configuración del pin del sensor de contacto.
  - Rutina de interrupción (ISR) y lógica de *debounce*.
  - Señales al módulo de FSM cuando hay una nueva moneda.
- **`GimmiCoin_Balanza.ino`**
  - Inicialización del **HX711**.
  - Rutinas de *tare*, lectura múltiple y filtrado.
  - Conversión de lecturas crudas a gramos.
- **`GimmiCoin_Motor.ino`**
  - Control del **motor DC** (encendido/apagado).
  - Aplicación del tiempo de activación (*pulse width*).
- **`GimmiCoin_Sonido.ino`**
  - Inicialización del **DFPlayer Mini**.
  - Funciones para reproducir efectos de sonido al registrar una moneda.
- **`GimmiCoin_Memoria.ino`**
  - Lectura/escritura de valores en EEPROM (por ejemplo, el total acumulado).
  - Estrategia de reducción de escrituras (timeout antes de persistir).
- **`GimmiCoin_Web.ino`**
  - Configuración del **Access Point**.
  - Manejo del servidor HTTP (`/`, `/estado`, `/admin`, `/reset`).
  - Generación de HTML y JSON.

Esta modularización facilita el desarrollo incremental, las pruebas por subsistema y el mantenimiento futuro.

---

## 3. Máquina de Estados (FSM)

El flujo de alto nivel del firmware se modela como una **FSM**. Las transiciones dependen de los eventos del sensor de moneda, la estabilidad del peso, el tiempo de espera y los comandos de la aplicación web.

### 3.1 Estados principales

1. **DeepSleep**  
   - El ESP8266 se encuentra en bajo consumo, esperando un evento externo.
2. **WakeInit**  
   - Inicialización básica: pines, HX711, DFPlayer, servidor web, lectura de total almacenado.
3. **Detect**  
   - Espera de detección de moneda por el sensor de contacto (bandera de ISR).
4. **Weigh**  
   - Lectura del peso con el HX711 y filtrado de muestras.
5. **Actions**  
   - Activación del motor DC y del DFPlayer (movimiento + sonido).
6. **UpdateEEPROM**  
   - Actualización del total en memoria si corresponde.
7. **WaitCoin**  
   - Ventana de tiempo para recibir otra moneda antes de dormir.
8. **PreSleep**  
   - Preparación para entrar nuevamente en `DeepSleep`.

### 3.2 Diagrama de Estados (Mermaid)

```mermaid
stateDiagram-v2
    [*] --> DeepSleep

    state "Deep Sleep" as DeepSleep
    state "Wake / Init" as WakeInit
    state "Detección de moneda" as Detect
    state "Pesaje (HX711)" as Weigh
    state "Acciones (motor / audio)" as Actions
    state "Actualizar EEPROM" as UpdateEEPROM
    state "Esperar otra moneda" as WaitCoin
    state "Pre-sleep" as PreSleep

    DeepSleep --> WakeInit: Wake por sensor / reset
    WakeInit --> Detect: Init HX711 / DFPlayer /\nrestaurar total
    Detect --> Weigh: Moneda detectada\n(bandera ISR)
    Weigh --> Actions: Peso estable y válido
    Actions --> UpdateEEPROM: Movimiento + sonido OK
    UpdateEEPROM --> WaitCoin: Persistir si timeout cumplido

    WaitCoin --> Detect: Nueva moneda\nantes del timeout
    WaitCoin --> PreSleep: Sin monedas\ndurante timeout

    PreSleep --> DeepSleep: Guardar estado mínimo\n+ entrar en deep sleep
````

---

## 4. Flujo del Programa Principal

El flujo general en `GimmiCoin_Main.ino` puede resumirse así:

```cpp
void setup() {
  initPins();
  initSensorMoneda();
  initBalanzaHX711();
  initDFPlayer();
  initWiFiAP();
  initWebServer();
  cargarTotalDesdeEEPROM();
  fsm_setState(WAKE_INIT);
}

void loop() {
  // Atender peticiones HTTP
  web_handleClient();

  // Actualizar FSM
  fsm_update();

  // Otras tareas periódicas (si aplica)
}
```

La función `fsm_update()` revisa el estado actual, procesa los eventos (moneda detectada, peso estable, timeout) y realiza las transiciones correspondientes.

---

## 5. Estrategia de Gestión de E/S

### 5.1 Sensor de moneda – Interrupción

El **sensor de contacto** se usa como disparador principal; se conecta a un pin digital con un **pull-down externo** y se configura para generar una interrupción cuando hay cambio de estado.

* Modo: interrupción por cambio (`CHANGE`) o flanco descendente (`FALLING`), según el diseño final.
* Objetivo: detectar el momento en que la moneda cierra el circuito.
* La lógica pesada (pesaje, sonido, motor) **no se ejecuta en la ISR**, solo se marca una bandera.

Ejemplo simplificado de ISR:

```cpp
volatile bool monedaDetectada = false;
volatile unsigned long lastPulseMicros = 0;
const unsigned long MIN_PULSE_INTERVAL = 300000; // 0.3 s

void IRAM_ATTR isrSensorMoneda() {
  unsigned long now = micros();
  if (now - lastPulseMicros > MIN_PULSE_INTERVAL) {
    monedaDetectada = true;       // Se procesa luego en la FSM
    lastPulseMicros = now;
  }
}
```

En `loop()`, la FSM consulta `monedaDetectada` y, si está activa, la consume y pasa al estado `Weigh`.

### 5.2 Lectura de balanza – Polling controlado

El módulo **HX711** se lee mediante una interfaz sincrónica de 2 hilos (DT/SCK). El acceso se implementa mediante polling controlado:

* La rutina de balanza:

  * Obtiene varias lecturas consecutivas.
  * Descartar outliers.
  * Calcula un promedio.
* El peso final se pasa a la FSM para clasificación por rangos.

Ejemplo de flujo lógico:

```cpp
float leerPesoFiltrado() {
  const int N = 10;
  long suma = 0;
  for (int i = 0; i < N; i++) {
    long lectura = hx711.read();
    suma += lectura;
  }
  long promedio = suma / N;
  return convertirALibrasOGramos(promedio); // según la calibración
}
```

La FSM asegura que el motor no se active hasta que el peso sea estable.

### 5.3 Motor DC – Salida digital

El **motor DC** se controla con un solo pin digital (por ejemplo `D8`), que gobierna la base del 2N2222:

* Una función de alto nivel, por ejemplo `motor_pulse(T_MOTOR_MS)`, se encarga de:

  * Activar el pin.
  * Esperar el tiempo necesario.
  * Apagar el pin.

Esta operación se llama desde el estado `Actions` de la FSM.

### 5.4 DFPlayer Mini – UART

El DFPlayer se controla mediante UART (hardware o `SoftwareSerial`):

* Comandos básicos: inicialización, selección de track, volumen y reproducción.
* La capa de servicio (`GimmiCoin_Sonido.ino`) expone funciones como:

```cpp
void sonido_init();
void sonido_reproducirMoneda();
```

Estas se invocan desde la FSM en el estado `Actions`.

### 5.5 Servidor Web – HTTP

El servidor HTTP se implementa con `ESP8266WebServer`:

* Se atiende dentro de `loop()` mediante `web_handleClient()`.
* El tiempo de atención es corto para no bloquear la FSM.

---

## 6. Prioridades, Latencias y Sincronización

### 6.1 Prioridades

* **Máxima prioridad:** ISR del sensor de moneda.

  * Debe ser extremadamente corta: solo marcar banderas y tiempos.
* **Segundo nivel:** lectura de balanza (Weigh).

  * Se ejecuta cuando hay moneda detectada; durante este proceso el motor y el DFPlayer se mantienen inactivos.
* **Tercer nivel:** servidor web HTTP.

  * Atiende peticiones entre eventos de la FSM, sin bloquear el flujo principal.
* **Cuarto nivel:** escritura en EEPROM.

  * Se realiza de forma diferida, después de un timeout, para no afectar la respuesta del sistema.

### 6.2 Latencias

* **Detección de moneda:** inmediata (interrupción) con validación de *debounce* por tiempo.
* **Pesaje:** la obtención de un peso estable puede tomar decenas de milisegundos, debido al muestreo múltiple y al tiempo de estabilización mecánica.
* **Acciones (motor y audio):** el tiempo de motor es fijo (`T_MOTOR`), el audio corre en paralelo.

La FSM está diseñada para que:

* El sistema responda rápidamente a la inserción de una moneda.
* No se mezclen acciones de varios eventos de moneda; se procesa una a la vez.

### 6.3 Sincronización

* Variables compartidas entre ISR y programa principal (`monedaDetectada`, `lastPulseMicros`) se declaran como `volatile`.
* La FSM consume y limpia las banderas de evento de manera atómica (o con secciones críticas muy pequeñas si se requiere).
* Las rutinas de EEPROM usan contadores/timers para evitar escrituras excesivas.

---

## 7. Protocolos de Comunicación

### 7.1 Interfaz con HX711 (tipo SPI de 2 hilos)

* El HX711 utiliza una interfaz síncrona de 2 hilos (`DT` y `SCK`).
* El ESP8266 genera los pulsos de reloj en `SCK` y lee datos en `DT`.
* La implementación es *bit-banged* (no se usa el SPI hardware), pero desde el punto de vista de arquitectura se considera un **protocolo digital síncrono**.

### 7.2 UART con DFPlayer

* Comunicación serial asíncrona a 9600 bps.
* Protocolo de comandos propio del DFPlayer (paquetes con encabezados, longitud y checksum).
* Permite controlar la reproducción de pistas de audio almacenadas en la tarjeta microSD.

### 7.3 HTTP sobre WiFi (SoftAP)

* El ESP8266 crea una red **Access Point** (SoftAP).
* Los clientes se conectan con su teléfono o portátil.
* La comunicación se realiza vía HTTP:

  * `GET /` → página HTML con el estado de la alcancía.
  * `GET /estado` → JSON con información para clientes más avanzados.
  * `GET /admin` y `GET /reset` → gestión de la alcancía (protegido).

Esta combinación de protocolos cumple con el requisito del curso de utilizar **múltiples protocolos de comunicación digitales** (UART, interfaz tipo SPI y HTTP sobre WiFi).

---

## 8. Organización de Memoria

### 8.1 RAM

* Variables principales:

  * Contadores de monedas y montos.
  * Estado actual de la FSM.
  * Buffers temporales para lecturas de HX711.
  * Buffers pequeños de strings para HTML/JSON.
* Optimización:

  * Se evita el uso de `String` dinámico en bucles críticos.
  * Se prefiere `String` local de tamaño acotado o cadenas `const char*` almacenadas en flash (`PROGMEM`) cuando es posible.

### 8.2 EEPROM / Flash

* Se usa EEPROM para almacenar:

  * El total acumulado (CRC o colones).
* Estrategia de escritura:

  * Cada vez que se detecta una moneda, se **actualiza el total en RAM**.
  * Un **timeout** (ej. 12 s sin nuevas monedas) dispara la escritura en EEPROM.
  * De esta forma se reducen las escrituras y se alarga la vida útil de la memoria.

### 8.3 Código (Flash)

* El firmware completo se almacena en la flash del ESP8266.
* Las páginas web (HTML, CSS y posible JavaScript ligero) se incrustan como literales de texto en el código:

  * Se usan `R"rawliteral(...)"` para reducir el overhead de concatenaciones.
  * Esto permite servir la interfaz sin necesidad de un sistema de archivos adicional.

---
