# Definition of Done (DoD)

**Proyecto:** Gimmighoul Coin Collector Robot  
**Curso:** CE4301 – Arquitectura de Computadores I  
**Instituto Tecnológico de Costa Rica**  
**Fecha:** Noviembre 2025  

---

## 1. Propósito

La *Definition of Done* (DoD) establece los criterios específicos que determinan cuándo una funcionalidad o entregable del proyecto se considera **completamente implementado y validado**.  
Su objetivo es garantizar uniformidad, trazabilidad y calidad en todas las fases del desarrollo del sistema: electrónica, firmware, aplicación móvil y documentación.

Cada funcionalidad se considerará **terminada** únicamente cuando cumpla con todos los criterios generales y específicos descritos a continuación.

---

## 2. Criterios Generales de Finalización

Una tarea, issue o historia de usuario se considera *Done* cuando:

1. El código o montaje **compila y funciona correctamente** sin errores críticos.  
2. Se han **realizado pruebas unitarias o de integración** que validan su comportamiento.  
3. El componente (hardware o software) está **correctamente documentado** en el repositorio (`/docs` o README del módulo).  
4. La funcionalidad está **integrada en el sistema principal** y opera de forma coherente con los demás módulos.  
5. Se ha verificado la **compatibilidad eléctrica y lógica** (3.3 V o 5 V según corresponda).  
6. La funcionalidad cumple los **requisitos definidos en los issues** asociados al sprint correspondiente.  
7. El resultado ha sido **revisado por al menos un miembro del equipo** y validado durante la reunión de cierre del sprint.  
8. La tarea está **marcada como “Closed”** en GitHub con comentarios o evidencia de pruebas (foto, video, salida serial, etc.).

---

## 3. Criterios Específicos por Subsistema

### 3.1 Firmware (Microcontrolador ESP8266)

- El código fuente se encuentra organizado y comentado en la carpeta `/firmware`.  
- El firmware permite:
  - Leer correctamente la celda de carga a través del **HX711** (comunicación SPI).  
  - Detectar la presencia de monedas mediante el **sensor infrarrojo**.  
  - Controlar los **dos servomotores SG90** con movimientos suaves y precisos.  
  - Activar el **DFPlayer Mini** para reproducir audio tras detectar una moneda.  
- El firmware arranca correctamente al encender la alimentación sin necesidad de conexión USB.  
- Se han validado los tiempos de respuesta y estabilidad energética del sistema.  

### 3.2 Sensado y Detección (HX711 + IR)

- La **celda de carga** mide el peso de las monedas con una **precisión mínima del ±5 %**.  
- El **sensor infrarrojo** detecta de forma confiable la inserción de la moneda (> 95 % de efectividad).  
- La lectura digital SPI entre el HX711 y el ESP8266 es estable, sin interferencias ni lecturas falsas.  
- Se ha documentado la calibración de la celda de carga (procedimiento y valores finales).  

### 3.3 Actuadores (Servos SG90)

- El **servo principal** levanta la tapa del cofre sin obstrucciones ni sobrecarga.  
- El **servo auxiliar** empuja o limpia la moneda de la celda de carga correctamente tras cada medición.  
- Ambos servos se alimentan de la línea de 5 V regulada y no generan caída de tensión perceptible.  
- Se realizaron pruebas de repetición (> 20 ciclos) sin fallos mecánicos o eléctricos.  

### 3.4 Audio (DFPlayer Mini + Parlante)

- El módulo DFPlayer Mini reproduce correctamente un archivo de audio en formato `.mp3` almacenado en la microSD.  
- La conexión con el parlante de **8 Ω / 0.5 W** genera un volumen adecuado sin distorsión.  
- El audio se activa automáticamente cuando se detecta una moneda.  
- La comunicación UART entre el DFPlayer y el ESP8266 responde sin errores.  

### 3.5 Alimentación y Regulación (TP4056, MT3608, LM2596)

- El sistema se alimenta de la **batería Li-Ion de 3.7 V / 2000 mAh**, la cual brinda al menos **30 minutos de operación continua**.  
- El **TP4056** permite la recarga segura de la batería mediante mini-B, con indicador de carga.  
- El **MT3608** eleva correctamente la tensión a 5 V y el **LM2596** la reduce a 3.3 V estables.  
- Se verifican las tensiones en carga y descarga con un multímetro antes de cerrar la etapa de pruebas.  
- Los condensadores de filtrado (470 µF en 5 V y 100 µF en 3.3 V) están instalados y operativos.  

### 3.6 Comunicación y Aplicación Móvil

- El ESP8266 crea una red WiFi local y transmite los datos de ahorro al cliente móvil.  
- La aplicación móvil recibe, interpreta y muestra correctamente el monto total acumulado.  
- La actualización del valor ocurre en **tiempo real** al insertar una moneda.  
- Se han realizado pruebas de conexión y desconexión sin pérdida de datos.  

### 3.7 Estructura Física y Ensamble

- Todos los módulos están montados en una **placa perforada o base fija** con cableado ordenado.  
- El diseño físico del **Pokémon Gimmighoul** (cofre) fue impreso en 3D o construido artesanalmente según especificaciones.  
- La tapa abre y cierra correctamente durante las pruebas funcionales.  
- El sistema opera sin conexión a la computadora (autónomo con batería).  

### 3.8 Documentación y Entrega Final

- Se incluyen en el repositorio:
  - Diagramas eléctricos y de conexión.  
  - Código fuente del firmware y aplicación móvil.  
  - Fotografías del prototipo final ensamblado.  
  - Resultados de pruebas y calibraciones.  
- El archivo **README.md** del repositorio principal contiene:
  - Instrucciones de montaje.  
  - Modo de uso del sistema.  
  - Créditos del equipo y licenciamiento.  

---

## 4. Validación Final del Proyecto

El proyecto se considera **completamente finalizado** cuando:

1. Todos los **issues** y **sub-issues** de los sprints 0–4 están cerrados.  
2. El sistema es capaz de **detectar, pesar y registrar** monedas de forma continua sin intervención externa.  
3. Se reproduce el sonido del Gimmighoul al insertar una moneda.  
4. Los datos del monto total se visualizan correctamente en la aplicación móvil.  
5. El circuito y la carcasa física se encuentran ensamblados y funcionales.  
6. Toda la documentación técnica y de usuario está disponible en el repositorio.

---

## 5. Revisión y Aprobación

| **Versión** | **Fecha** | **Responsables** | **Descripción** |
|--------------|------------|------------------|------------------|
| 1.0 | 05/11/2025 | José B. Barquero, J. E. Campos, J. Feng, A. Montero | Versión inicial de la Definition of Done para el proyecto Gimmighoul Coin Collector Robot. |

---
