# Diseño de Hardware – GimmiCoin (Alcancía Inteligente)

## 1. Selección y Justificación del Microcontrolador

El sistema GimmiCoin está basado en el **SoC ESP8266**, utilizando el módulo **D1 Mini** como plataforma de integración. La elección de este microcontrolador se justifica por las siguientes razones:

- **Conectividad Wi-Fi integrada:** permite implementar un servidor web embebido y operar en modo *Access Point* sin hardware adicional de red.
- **Capacidad de cómputo adecuada:** el ESP8266 ofrece recursos suficientes para:
  - Manejar interrupciones del sensor de moneda.
  - Leer y filtrar datos del módulo **HX711**.
  - Controlar el **motor DC** para la limpieza de monedas.
  - Gestionar la comunicación HTTP con la interfaz web.
- **Ecosistema maduro de librerías:** existen librerías probadas para:
  - Servidor web (`ESP8266WebServer`).
  - Manejo de EEPROM.
  - Integración con el módulo **HX711**.
  - Control del módulo de audio **DFPlayer Mini**.
- **Factor de forma compacto:** el D1 Mini tiene un tamaño reducido, lo cual facilita su integración dentro de la estructura física del personaje Gimmighoul.
- **Regulación de 3.3 V integrada:** el propio módulo incorpora regulación a 3.3 V desde 5 V, simplificando el diseño de alimentación cuando se usa una *powerbank* como fuente.

En el montaje final sobre **tarjeta perforada**, se replica el esquema de conexiones del D1 Mini con el resto de componentes, respetando niveles lógicos de 3.3 V y una distribución física adecuada para minimizar ruido y cableado excesivo.

---

## 2. Diagrama Eléctrico General del Sistema

El circuito completo del sistema se modeló y verificó en **Fritzing**. A continuación se muestra el diagrama eléctrico utilizado como referencia para el montaje en tarjeta perforada:

![Diagrama eléctrico del sistema](Circuito_PG02_CE4301_2025.png)

---

## 3. Sensores y Estrategia de Medición

### 3.1 Sensor de Moneda por Contacto

Para la detección básica de la inserción de una moneda se utiliza un **sensor de contacto metálico**:

* Implementado como un par de conductores metálicos que se cierran cuando la moneda pasa por el canal de entrada.
* Un extremo se conecta a **Vcc** (a través de adecuadas resistencias si se requiere) y el otro al pin digital configurado con un **pull-down** externo de 10 kΩ.
* El pin del ESP8266 (por ejemplo **D2**) se configura como entrada digital, leyendo un flanco limpio (paso de **NO contacto → contacto**) que dispara la lógica de conteo y pesaje.
* La detección se complementa con lógica de *debounce* por tiempo en firmware para evitar múltiples detecciones por una misma moneda.

Este sensor permite un disparo confiable del flujo de la FSM sin requerir sensores ópticos adicionales.

### 3.2 Celda de Carga TAL221 + HX711

La **celda de carga TAL221 (100 g)** se utiliza para obtener el peso de la moneda y con ello inferir su denominación:

* La celda funciona como un puente de Wheatstone de 4 hilos, que se conecta al módulo **HX711**.
* El HX711 amplifica la señal analógica de la celda y la convierte a digital mediante un ADC de 24 bits.
* Conexión típica al ESP8266:

  * `DT` → pin digital (ej. **D5**).
  * `SCK` → pin digital (ej. **D6**).
  * `VCC` → 3.3 V.
  * `GND` → GND común.
* En firmware se realiza:

  * Secuencia de *tare* al iniciar el sistema (con la balanza vacía).
  * Lectura de múltiples muestras y promediado para obtener un peso estable.
  * Comparación del peso con rangos predefinidos para clasificar el tipo de moneda (p. ej. ~2.2 g, ~3.5 g, ~7.0 g, ~10.1 g).

Esta estrategia permite distinguir monedas con diámetros similares pero pesos diferentes, mejorando la confiabilidad en la clasificación.

---

## 4. Actuadores y Etapa de Potencia

### 4.1 Motor DC para Limpieza de Monedas

El sistema emplea un **motor DC** como único actuador mecánico para:

* Desplazar la moneda fuera de la zona de pesaje una vez completida la lectura de peso.
* Evitar acumulación de monedas sobre la celda, lo que podría causar errores de medición.

El motor se controla mediante un **transistor NPN 2N2222** en configuración de conmutación:

* **Base**: conectada a un pin digital (por ejemplo **D8**) mediante una resistencia en serie (ej. 1 kΩ).
* **Colector**: al terminal negativo del motor.
* **Emisor**: a GND.
* El terminal positivo del motor se conecta directamente a la línea de **5 V** de la powerbank.
* En paralelo con el motor se coloca un **diodo flyback** (ej. 1N4148 o 1N4007) para proteger contra picos de tensión inducidos al apagar el motor.

Este arreglo forma una etapa de potencia sencilla y adecuada para el consumo típico del motor, aprovechando la capacidad de corriente de la powerbank.

---

## 5. Módulo de Audio: DFPlayer Mini + Parlante

Para la retroalimentación sonora al usuario se utiliza un **DFPlayer Mini**, controlado desde el ESP8266:

* Comunicación mediante UART:

  * `TX` del ESP8266 → `RX` del DFPlayer (a través de una **resistencia serie de 1 kΩ** para adaptar niveles y proteger la entrada).
  * `RX` del ESP8266 → `TX` del DFPlayer (según diseño y disponibilidad de pines).
* Alimentación:

  * `VCC` → 5 V (desde la powerbank).
  * `GND` → GND común.
* El módulo se conecta a un **parlante de 8 Ω / 0.5 W**, suficiente para reproducir efectos de sonido al registrar monedas.

---

## 6. Alimentación y Distribución de Energía

La alimentación del sistema se basa en una **powerbank de 5 V**:

* El **D1 Mini** integra un regulador a 3.3 V, por lo que puede alimentarse directamente desde los 5 V de la powerbank.
* El motor DC y el DFPlayer Mini se alimentan también desde la línea de **5 V**.
* El módulo HX711 se alimenta desde la línea de **3.3 V** del D1 Mini, compartiendo GND común.

---

## 7. Consumo Energético Estimado

El consumo estimado de cada bloque es:

| Componente               | Consumo aproximado             |
| ------------------------ | ------------------------------ |
| ESP8266 (D1 Mini)        | 70–200 mA (según WiFi y carga) |
| HX711 + celda de carga   | ~10 mA                         |
| DFPlayer Mini + parlante | 40–100 mA (según volumen)      |
| Motor DC                 | 150–300 mA (ráfagas cortas)    |

En operación típica, el consumo promedio se mantiene dentro de la capacidad de salida de una powerbank estándar (5 V, 2–3 A), permitiendo sesiones prolongadas de uso. La activación del motor DC ocurre solo durante breves intervalos después de cada moneda, lo que reduce su impacto en el consumo medio.

---

## 8. Tarjeta Perforada y Organización Física

Todo el circuito se implementa sobre una **tarjeta perforada** en la versión final del proyecto, cumpliendo con la especificación del curso:

* Se ubican el D1 Mini, el módulo HX711, el transistor 2N2222, el diodo flyback, las resistencias y conectores hacia el motor, sensor de contacto, DFPlayer y parlante.
* Se organizan las pistas de alimentación (5 V y GND) en rieles bien definidos para reducir la cantidad de cables sueltos.
* Las señales sensibles (DT/SCK del HX711 y la línea del sensor de contacto) se mantienen separadas del cableado del motor para minimizar el ruido.
* Se utilizan conectores o headers desmontables para facilitar el montaje y mantenimiento dentro de la estructura del Gimmighoul.

---

## 9. Consideraciones de Diseño Mecánico

El chasis del proyecto se basa en la estética del personaje **Gimmighoul**:

* La **bandeja de pesaje** se monta rígidamente sobre la celda TAL221, de forma que solo las fuerzas verticales de la moneda sean transmitidas a la celda.
* El **motor DC** y su mecanismo (por ejemplo, una paleta/plataforma que empuja la moneda) se alinean con la salida de la bandeja para que todas las monedas abandonen la zona de medición.
* El **sensor de contacto** se ubica en la entrada de monedas, garantizando que cualquier moneda deba cerrar el circuito para ser detectada.
* Se reserva un compartimento aislado para:

  * Powerbank.
  * Tarjeta perforada.
  * Cableado.
  * DFPlayer y parlante.
* Se evita el contacto directo de las monedas con la electrónica para prevenir cortocircuitos y daños por suciedad o humedad.

