# Risk Assessment  

**Proyecto:** Gimmighoul Coin Collector Robot  
**Curso:** CE4301 – Arquitectura de Computadores I  
**Instituto Tecnológico de Costa Rica**  
**Fecha:** Noviembre 2025  

---

## 1. Objetivo del Análisis de Riesgos  

El presente documento tiene como finalidad identificar, analizar y mitigar los riesgos asociados al desarrollo del proyecto *Gimmighoul Coin Collector Robot*.  
Este sistema combina hardware embebido, control electrónico, comunicaciones inalámbricas y una interfaz móvil, por lo que su desarrollo implica riesgos tanto técnicos como operativos.  
El análisis se centra en tres dimensiones principales:

1. **Probabilidad de ocurrencia (P):** frecuencia o posibilidad de que el riesgo se materialice.  
2. **Impacto (I):** nivel de afectación al proyecto en caso de que ocurra el riesgo.  
3. **Severidad (S):** producto entre probabilidad e impacto (S = P × I).  

Cada riesgo se clasifica como **bajo (1–3)**, **medio (4–6)** o **alto (7–9)** según su severidad.

---

## 2. Matriz de Riesgos Identificados  

| **ID** | **Tipo de Riesgo** | **Descripción** | **Probabilidad (1–3)** | **Impacto (1–3)** | **Severidad (P×I)** | **Clasificación** | **Plan de Mitigación** |
|:------:|--------------------|------------------|:----------------------:|:----------------:|:-------------------:|------------------|-------------------------|
| R1 | **Hardware** | Daño en el microcontrolador Mini D1 Wemos ESP8266 por sobrecorriente durante picos de consumo de servos. | 2 | 3 | 6 | Medio | Incorporar capacitores de desacoplo (470 µF y 100 µF), fusible PTC de 3 A y diodo Schottky en la línea de alimentación. Verificar consumo antes de la integración final. |
| R2 | **Alimentación** | Descarga prematura o inestabilidad en la batería Li-Ion 3.7 V (2000 mAh) durante funcionamiento autónomo. | 3 | 2 | 6 | Medio | Calibrar MT3608 y LM2596 para evitar sobreconsumo. Realizar pruebas de autonomía antes de la demostración. Implementar control de apagado manual. |
| R3 | **Comunicación digital** | Fallos en la comunicación SPI entre el módulo HX711 y el ESP8266, afectando la lectura del peso. | 2 | 3 | 6 | Medio | Utilizar líneas cortas y con GND compartido. Añadir resistencias pull-down en DT/SCK si hay ruido. Validar tiempos de muestreo mediante pruebas previas. |
| R4 | **Sensores / Medición** | Lecturas erróneas de la celda de carga por vibración o interferencia de servos. | 2 | 3 | 6 | Medio | Montar la celda en superficie estable con amortiguación de goma. Filtrar lecturas en software y promediar muestras. |
| R5 | **Software / Firmware** | Desincronización entre la lectura de moneda, activación de servo y reproducción del sonido. | 2 | 3 | 6 | Medio | Implementar una máquina de estados bien definida y depurar cada módulo individual antes de la integración total. |
| R6 | **Aplicación móvil** | Problemas en la conexión WiFi entre el ESP8266 y la aplicación móvil (fallos de red o pérdida de datos). | 2 | 2 | 4 | Medio | Realizar pruebas de conexión local y reconexión automática en caso de pérdida. Optimizar el envío de datos con temporización controlada. |
| R7 | **Integración física** | Dificultades para ensamblar todos los componentes en el cuerpo del Pokémon Gimmighoul (espacio reducido, vibración). | 3 | 2 | 6 | Medio | Diseñar el modelo 3D con compartimentos modulares. Verificar medidas de servos y celda antes de imprimir. Usar impresión 3D en varias partes desmontables. |
| R8 | **Audio / DFPlayer** | Ruido o distorsión en la reproducción de sonido debido a alimentación inestable o conexión deficiente al parlante. | 2 | 2 | 4 | Medio | Filtrar la línea de 5 V con condensador de 470 µF. Verificar la impedancia del parlante (8 Ω). Asegurar soldaduras firmes. |
| R9 | **Calibración** | Desviaciones en la lectura del peso que impidan distinguir entre monedas de valores cercanos. | 3 | 3 | 9 | Alto | Realizar calibración por software con pesos conocidos. Implementar tabla de referencia y validación estadística. Repetir pruebas tras el montaje final. |
| R10 | **Gestión de tiempo** | Retrasos en la integración de hardware y software por dependencias entre módulos. | 2 | 3 | 6 | Medio | Planificar desarrollo por sprints (ya definidos). Priorizar firmware base y pruebas funcionales antes de estética. |
| R11 | **Documentación / Entregables** | Falta de evidencia visual o técnica completa para la entrega final. | 2 | 2 | 4 | Medio | Mantener documentación paralela al desarrollo. Incluir fotografías, diagramas y video final de demostración. |

---

## 3. Análisis Global de Riesgos  

- **Total de riesgos identificados:** 11  
- **Distribución de severidad:**  
  - 1 riesgo alto (R9)  
  - 9 riesgos medios  
  - 1 riesgo bajo (implícito en la operación normal del sistema)

**Riesgos críticos:**  
El riesgo más significativo es el **R9 (calibración del peso)**, ya que determina la precisión principal del sistema.  
Una desviación mínima puede generar conteos erróneos, afectando la funcionalidad central del proyecto.

**Riesgos de segundo nivel:**  
Los asociados a **alimentación (R2)**, **integración física (R7)** y **comunicación digital (R3)**, los cuales pueden comprometer el funcionamiento autónomo del sistema si no se controlan adecuadamente.

---

## 4. Estrategias Generales de Mitigación  

1. **Pruebas incrementales:** validar cada módulo (celda, servo, DFPlayer) antes de la integración total.  
2. **Gestión energética:** medir consumo real y ajustar voltajes con multímetro antes de conectar al sistema final.  
3. **Calibración inicial y revalidación:** registrar pesos de referencia y comparar resultados luego del montaje.  
4. **Control de versiones:** documentar cambios de firmware y conexión eléctrica en el repositorio GitHub.  
5. **Redundancia de tiempo:** mantener al menos 3 días antes de la entrega final para pruebas, ajustes y grabación del video demostrativo.  

---

## 5. Conclusión  

El análisis de riesgos demuestra que el proyecto es **viable y de complejidad media**, con la mayoría de riesgos bajo control mediante buenas prácticas de integración hardware–software.  
La planificación por sprints y la modularidad del diseño permiten detectar y corregir fallos tempranamente, asegurando la entrega de un prototipo funcional, seguro y bien documentado.

---
