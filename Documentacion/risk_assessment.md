# Risk Assessment – Proyecto GimmiCoin  

---

## 1. Objetivo del Análisis de Riesgos

Este documento identifica, valora y propone estrategias de mitigación para los riesgos técnicos relacionados con el desarrollo del sistema **GimmiCoin**, una alcancía inteligente basada en ESP8266, sensor de contacto, celda de carga + HX711, motor DC, DFPlayer Mini y comunicación mediante aplicación web.

El análisis considera:

- **Probabilidad (P):** frecuencia esperada del riesgo (1–3).  
- **Impacto (I):** severidad potencial sobre el proyecto (1–3).  
- **Severidad total (S = P × I):**  
  - 1–3 → Bajo  
  - 4–6 → Medio  
  - 7–9 → Alto  

---

## 2. Matriz de Riesgos Actualizada

| **ID** | **Tipo** | **Descripción del riesgo** | **P** | **I** | **S** | **Severidad** | **Plan de mitigación** |
|-------|----------|----------------------------|------|------|------|----------------|--------------------------|
| R1 | Hardware | Ruido eléctrico generado por el motor DC afectando las lecturas del HX711. | 2 | 3 | 6 | Medio | Colocar diodo flyback, condensadores de desacoplo (470–1000 µF), y separar rutas de GND. Leer balanza solo cuando el motor esté apagado. |
| R2 | Sensores | Lecturas inestables del HX711 debido a vibraciones o mala fijación de la celda TAL221. | 3 | 3 | 9 | Alto | Montaje rígido de la celda, amortiguación mecánica, lecturas promediadas (10–12 muestras), aislamiento del motor. |
| R3 | Sensores | Rebotes del sensor de contacto que generen múltiples detecciones falsas. | 3 | 2 | 6 | Medio | ISR mínima + *debounce* por tiempo, validación de intervalo mínimo entre pulsos, filtrado software. |
| R4 | Software | Desincronización entre FSM, pesaje y activación del motor (procesamiento de múltiples monedas simultáneas). | 2 | 3 | 6 | Medio | Procesar monedas de una en una en la FSM, bloquear eventos mientras se pese, añadir banderas y estados seguros. |
| R5 | Firmware | Fallos en comunicación UART con DFPlayer (no inicia, no reproduce). | 2 | 2 | 4 | Medio | Añadir retardo inicial (≥1200 ms), verificar resistencia serie de 1 kΩ, revisar soldaduras, reintento de comandos. |
| R6 | Comunicación | Pérdida de conexión WiFi del usuario al AP durante el uso. | 1 | 2 | 2 | Bajo | Usar SoftAP estable, controlar reconexiones, minimizar peso de páginas HTML. |
| R7 | Alimentación | Caídas de tensión al activar el motor si la powerbank no entrega suficiente corriente. | 2 | 3 | 6 | Medio | Usar powerbank ≥2 A, añadir condensadores de reserva, cables cortos y gruesos, pruebas de carga con motor activo. |
| R8 | Integración | Espacio reducido dentro del Gimmighoul para acomodar perforada, cables, motor y balanza. | 3 | 2 | 6 | Medio | Organizar cableado, usar conectores desmontables, modelar compartimentos, fijar electrónica en capas. |
| R9 | Firmware | Escrituras excesivas en EEPROM reduciendo su vida útil. | 2 | 2 | 4 | Medio | Implementar escritura diferida por timeout (12 s sin monedas), mantener el total en RAM durante operación. |
| R10 | Aplicación web | Fallos en endpoints `/reset` o `/estado` bajo carga o múltiples clientes. | 1 | 2 | 2 | Bajo | Manejar correctamente respuestas JSON, sanitizar inputs, priorizar FSM sobre webserver. |
| R11 | Medición | Error en calibración del peso que cause clasificación incorrecta de monedas. | 3 | 3 | 9 | **Alto** | Calibración con pesos reales, tablas de referencia, verificación después del ensamblaje final, pruebas repetitivas. |
| R12 | Pruebas | Dificultad para reproducir condiciones reales de uso bajo tiempo limitado. | 2 | 2 | 4 | Medio | Crear un plan de pruebas secuenciales (sensor, balanza, motor, audio, FSM, web), registrar resultados en video. |

---

## 3. Análisis Global de Riesgos

- **Total de riesgos identificados:** 12  
- **Distribución por severidad:**  
  - **2 riesgos altos (R2, R11)**  
  - **8 riesgos medios**  
  - **2 riesgos bajos**

### Riesgos críticos
1. **R2 – Vibración y mala fijación de celda de carga (ALTO)**  
   Puede causar lecturas inestables que afectan toda la clasificación.

2. **R11 – Calibración del peso incorrecta (ALTO)**  
   Un error en calibración hace que el robot registre monedas equivocadas o inconsistentes.

Ambos riesgos afectan directamente el propósito del proyecto (clasificación por peso).

### Riesgos importantes (medios)
- Ruido del motor (R1)  
- Rebotes del sensor (R3)  
- Sincronización FSM–motor–balanza (R4)  
- Caídas de alimentación (R7)  
- Integración física (R8)

Todos estos se han observado durante el desarrollo y requieren verificación final cerca del prototipo terminado.

### Riesgos menores
- Pérdida WiFi ocasional (R6)  
- Problemas en página web o JSON (R10)  
- Sobreescritura de EEPROM (R9)  

Estos son simples de mitigar con pruebas y ajustes menores.

---

## 4. Estrategias generales de mitigación

1. **Pruebas incrementales por módulos**  
   - Sensor → Balanza → Motor → DFPlayer → Web → FSM.  
   - Nunca integrar piezas sin probar cada módulo por separado primero.

2. **Aislamiento eléctrico y mecánico**  
   - Condensadores grandes en 5 V y 3.3 V.  
   - Separar físicamente motor y celda.  
   - Cableado ordenado y firme.

3. **Calibración rigurosa del HX711**  
   - Usar pesos patrón o monedas reales conocidas.  
   - Hacer calibración después de integrar la mecánica final.  
   - Mantener condiciones estables durante la toma de lecturas.

4. **FSM robusta**  
   - Procesar solo una moneda a la vez.  
   - Bloquear pesos durante acciones mecánicas.  
   - Validar todas las transiciones.

5. **EEPROM con escritura diferida**  
   - Solo guardar cuando el usuario deja de insertar monedas.  
   - Reducir desgaste y evitar corrupción de datos.

6. **Simulación de uso real**  
   - Pruebas continuas insertando varias monedas rápido.  
   - Evaluar si ocurre ruido, fallos de detección, problemas con motor o pesaje.

---

## 5. Conclusión

El análisis de riesgos actualizado muestra que **GimmiCoin es técnicamente viable**, con riesgos moderados y dos riesgos críticos relacionados exclusivamente con la precisión del pesaje (vibración y calibración).  
Estos riesgos son **manejables** mediante buenas prácticas de diseño mecánico, promediado de lecturas, calibración adecuada y control temporal desde la FSM.

El resto de riesgos se mitiga adecuadamente con prueba continua, buen manejo de energía y separación de módulos.  
Con las estrategias aplicadas, el equipo puede entregar un prototipo final estable, funcional y coherente con los objetivos del curso.

