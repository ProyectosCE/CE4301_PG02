# Definition of Done (DoD) – Proyecto GimmiCoin

---

## 1. Propósito

La *Definition of Done* (DoD) define los criterios concretos que se deben cumplir para considerar que:

- Una **funcionalidad**,  
- Un **módulo** de hardware/firmware,  
- O un **entregable** del proyecto  

están **completamente terminados** y listos para demostración / defensa.

La DoD aplica a todos los subsistemas:

- Hardware (sensor de contacto, celda de carga + HX711, motor DC, DFPlayer, powerbank, perforada).  
- Firmware (FSM, lectura de sensores, control de actuadores, EEPROM).  
- Aplicación web (AP WiFi, endpoints HTTP, UI).  
- Documentación (repositorio, diagramas, pruebas, video).

---

## 2. Criterios Generales de Finalización

Una historia de usuario, tarea o issue se considera **DONE** cuando:

1. El **código compila sin errores** y el firmware corre de forma estable en el ESP8266.  
2. El módulo de hardware está **correctamente cableado y probado** de forma independiente.  
3. Se han realizado **pruebas unitarias** o de integración que verifiquen el comportamiento esperado.  
4. La funcionalidad está **integrada en la FSM** y no rompe otros estados o flujos.  
5. Se ha actualizado la **documentación correspondiente** en el repositorio (`/docs`, `README`, comentarios de código, etc.).  
6. Existen **evidencias** (fotos, capturas de la web, videos cortos o salidas por puerto serial) vinculadas al issue.  
7. Al menos un miembro distinto del autor ha **revisado y aprobado** los cambios (revisión de código / pruebas en hardware).  
8. La tarea o issue aparece en GitHub como **Closed**, con una referencia a la rama / commit asociado.  

> Ninguna tarea se considera *DONE* solo porque “funciona en mi máquina”: debe pasar por integración, documentación mínima y evidencias.

---

## 3. Criterios Específicos por Subsistema

### 3.1 Firmware (FSM y lógica de control)

La parte de firmware se considera **terminada** cuando:

- La **FSM** implementa todos los estados definidos:  
  `DeepSleep`, `WakeInit`, `Detect`, `Weigh`, `Actions`, `UpdateEEPROM`, `WaitCoin`, `PreSleep`.  
- Se han probado **todas las transiciones** relevantes:
  - Inserción de moneda simple.  
  - Inserción de varias monedas seguidas.  
  - Timeout sin monedas (paso a `PreSleep` y luego `DeepSleep`).  
- El **código está modularizado** en los archivos `.ino` acordados:
  - `GimmiCoin_Main.ino` (setup, loop, FSM).  
  - `GimmiCoin_SensorMoneda.ino` (detección).  
  - `GimmiCoin_Balanza.ino` (HX711).  
  - `GimmiCoin_Motor.ino` (motor DC).  
  - `GimmiCoin_Sonido.ino` (DFPlayer).  
  - `GimmiCoin_Memoria.ino` (EEPROM).  
  - `GimmiCoin_Web.ino` (AP + HTTP).  
- No hay **bloqueos** ni reinicios inesperados durante al menos:
  - 10 ciclos completos de inserción de moneda → pesaje → motor → sonido → actualización.

---

### 3.2 Sensado de Moneda (Sensor de contacto)

El subsistema de **detección de moneda** se considera DONE cuando:

- El sensor de contacto está correctamente instalado en el canal de entrada de monedas.  
- El pin del ESP8266:
  - Está configurado como entrada digital con **pull-down externo** de 10 kΩ.  
  - Genera una interrupción (ISR) ante el flanco configurado.  
- La ISR:
  - Es **muy corta** (solo marca una bandera y registra `micros()`).  
  - Implementa un **debounce por tiempo** (`MIN_PULSE_INTERVAL`) para evitar rebotes.  
- La FSM solo procesa **una detección por moneda**, incluso cuando la moneda rebota mecánicamente.  
- En pruebas con al menos 20 monedas:
  - La tasa de detecciones falsas (sin moneda) es ~0%.  
  - La tasa de monedas perdidas es ≤ 5%.

---

### 3.3 Pesaje y Clasificación (Celda de carga + HX711)

El subsistema de **pesaje** se considera DONE cuando:

- La celda TAL221 está firmemente montada en la estructura del Gimmighoul, sin juego lateral excesivo.  
- El módulo HX711:
  - Está correctamente cableado (DT/SCK a pines del D1 Mini, VCC 3.3 V, GND común).  
  - Usa una frecuencia de muestreo estable (10–80 SPS).  
- Se realizó una **calibración documentada**:
  - Incluye factores de conversión de lectura cruda → gramos.  
  - Usa al menos dos pesos de referencia (monedas reales).  
- La función de lectura:
  - Promedia varias muestras (`N ≥ 10`).  
  - Aplica filtros básicos (por ejemplo, descartar outliers).  
- La clasificación:
  - Define rangos de peso claros para cada tipo de moneda soportada.  
  - Se probó con al menos 5 lecturas por tipo de moneda.  

Estado objetivo: **error de clasificación ≤ 5–10 %** en pruebas controladas.

---

### 3.4 Acciones Físicas: Motor DC

El subsistema del **motor DC** se considera DONE cuando:

- El motor está montado en la estructura de forma que:
  - Despeje completamente la moneda de la plataforma de pesaje.  
  - No interfiera físicamente con el sensor ni con la celda.  
- La etapa de potencia con **2N2222 + diodo flyback**:
  - Presenta conexiones correctas (base, colector, emisor, motor, diodo).  
  - Usa una resistencia base adecuada (por ejemplo 1 kΩ).  
- El firmware:
  - Implementa una función `motor_pulse(T_MOTOR_MS)` o equivalente.  
  - Garantiza que el motor se enciende por el tiempo correcto y luego se apaga.  
- En pruebas de ≥ 20 ciclos:
  - El motor no provoca reset del ESP8266.  
  - No induce errores severos en la lectura de la balanza (se lee solo con el motor apagado).  

---

### 3.5 Audio (DFPlayer Mini + Parlante)

El subsistema de **audio** se considera DONE cuando:

- El DFPlayer está correctamente cableado:
  - Alimentación a 5 V de la powerbank.  
  - GND común con el resto del sistema.  
  - Línea RX protegida con resistencia serie (≈1 kΩ).  
- La tarjeta microSD:
  - Contiene al menos un archivo `.mp3` correctamente numerado.  
- El firmware:
  - Inicializa el DFPlayer con un retardo suficiente (≥ 1200 ms).  
  - Permite reproducir el sonido de forma confiable ante cada moneda válida.  
- En pruebas de 20 monedas:
  - El sonido se reproduce > 90 % de las veces.  
  - No se escuchan artefactos graves por falta de alimentación (distorsión fuerte).  

---

### 3.6 Alimentación (Powerbank 5 V)

La **alimentación** del sistema se considera DONE cuando:

- El robot opera completamente desde una **powerbank de 5 V**, sin estar conectado al USB de la PC.  
- Se midieron aproximaciones de consumo en:
  - Reposo (sin monedas).  
  - Durante motores y DFPlayer.  
- No se presentan resets o parpadeos extraños al activar el motor o reproducir audio.  
- La autonomía demostrada es razonable para el uso esperado (por ejemplo, ≥ 30 minutos de uso continuo en demostración).

---

### 3.7 Persistencia (EEPROM / Flash)

El subsistema de **memoria persistente** se considera DONE cuando:

- El total acumulado (en colones o equivalentes) se guarda en EEPROM / Flash.  
- Se implementa una **estrategia de escritura diferida**:
  - Solo se escribe después de un tiempo sin nuevas monedas (ej. 12 s).  
  - Durante la inserción frecuente de monedas, solo se actualiza en RAM.  
- Al reiniciar el dispositivo:
  - El total se restaura correctamente desde EEPROM.  
- Se verificó que:
  - No se escribe en EEPROM en cada moneda.  
  - El número de escrituras se mantiene razonable frente a la vida útil estimada.

---

### 3.8 Aplicación Web y Comunicación WiFi

La **aplicación web** se considera DONE cuando:

- El ESP8266 crea un **Access Point** con SSID y password definidos.  
- Un teléfono o laptop puede:
  - Conectarse al AP.  
  - Abrir la URL `http://192.168.4.1/`.  
- Se implementan los endpoints:
  - `/` (HTML principal).  
  - `/estado` (JSON).  
  - `/admin` (panel admin, Basic Auth).  
  - `/reset` (reseteo del contador, Basic Auth).  
- La interfaz:
  - Muestra el total acumulado y el estado del sistema.  
  - Se actualiza periódicamente (por ejemplo, usando `fetch('/estado')` con `setInterval`).  
- La función de `reset`:
  - Reinicia el contador a 0.  
  - Actualiza EEPROM correctamente.  

Debe existir evidencia de uso real:
- Capturas de pantalla desde un teléfono.  
- Pruebas de conexión y reconexión sin cuelgues.

---

### 3.9 Estructura Física y Perforada

La parte **física** se considera DONE cuando:

- Todo el circuito electrónico está montado en una **tarjeta perforada** o PCB.  
- No hay conexiones sueltas o puentes temporales sobre protoboard.  
- El montaje mecánico dentro del Gimmighoul:
  - Permite cerrar la estructura.  
  - Protege la electrónica de contacto directo con las monedas.  
- Se cuenta con:
  - Fotografías del montaje interno.  
  - Fotografías del robot completo.

---

### 3.10 Documentación y Repositorio

La documentación se considera DONE cuando:

- El repositorio Git contiene:
  - `README.md` completo y actualizado.  
  - Carpeta `/docs` con:
    - Arquitectura hardware y software.  
    - Aplicación de usuario.  
    - BOM.  
    - Risk Assessment.  
    - Definition of Done (este documento).  
- Existen:
  - Diagramas en **Mermaid** o imágenes claras (diagramas de bloques, FSM).  
  - Evidencias de pruebas (fotos, capturas, video de demo).  
- Los issues principales están:
  - Cerrados.  
  - Enlazados a commits o Pull Requests.

---

## 4. Validación Final del Proyecto

El proyecto GimmiCoin se considera **completamente terminado** cuando:

1. Todas las historias de usuario planificadas para el alcance del curso están marcadas como DONE según los criterios anteriores.  
2. El robot:
   - Detecta monedas por contacto.  
   - Las pesa y clasifica correctamente (según calibres definidos).  
   - Acciona el motor para limpiar la bandeja.  
   - Reproduce audio con el DFPlayer.  
3. El total ahorrado puede verse en la aplicación web y se mantiene tras reinicios.  
4. El sistema funciona durante la **demostración completa** sin fallos críticos.  
5. Toda la documentación técnica y de usuario está disponible y sincronizada con el estado real del hardware y firmware.  

---

