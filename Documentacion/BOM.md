# Bill of Materials (BOM)

**Proyecto:** Gimmighoul Coin Collector Robot  
**Curso:** CE4301 – Arquitectura de Computadores I  
**Instituto Tecnológico de Costa Rica**

> **Base:** Alternativa B (versión con Mini D1 Wemos ESP8266, pesaje por celda de carga, sensor IR de presencia, audio con DFPlayer).  
> **Tipo de cambio de referencia (31/10/2025):** 1 USD = **₡502.01**

---

## 1) Microcontrolador

| **Componente** | **Descripción / Función** | **Especificaciones clave** | **Proveedor / Enlace** | **Costo (USD)** |
|---|---|---|---|---:|
| **Mini D1 Wemos ESP8266** | MCU principal: coordina lectura HX711, sensor IR, control de servos, audio y WiFi con la app. | Wi-Fi 2.4 GHz, 3.3 V lógico, UART / I²C / “SPI” (HX711 2-wire), USB para programación. | **Propio del equipo** | **0.00** |

---

## 2) Sensores

| **Componente** | **Descripción / Función** | **Especificaciones clave** | **Proveedor / Enlace** | **Costo (USD)** |
|---|---|---|---|---:|
| **Load Cell TAL221 (100 g Mini Straight Bar)** | Medición de peso de la moneda para inferir su denominación. | Capacidad 100 g, aluminio, puente Wheatstone 4 hilos. | MicroJPM | **16.95** |
| **HX711 Load Cell Amplifier** | ADC 24-bit y amplificador para la celda; interfaz digital 2 hilos (DT, SCK). | 2.7–5 V, 10/80 SPS, bajo ruido. | CRCibernética | **8.95** |

---

## 3) Actuadores

| **Componente** | **Descripción / Función** | **Especificaciones clave** | **Proveedor / Enlace** | **Costo (USD)** |
|---|---|---|---|---:|
| **Servo Motor SG90 (x2)** | (1) Levanta la tapa del cofre; (2) empuja/limpia la moneda tras el pesaje. | 5 V, 180°, torque aprox. 1.8 kg·cm. | MicroJPM | **5.95 × 2 = 11.90** |

---

## 4) Audio

| **Componente** | **Descripción / Función** | **Especificaciones clave** | **Proveedor / Enlace** | **Costo (USD)** |
|---|---|---|---|---:|
| **DFPlayer Mini (Módulo MP3)** | Reproduce efecto sonoro al registrar moneda (control por UART). | Soporta FAT16/32, salida SPK directa, 24-bit DAC. | CRCibernética | **4.95** |
| **Parlante 8 Ω / 0.5 W** | Salida de audio directa del DFPlayer. | Ø ~57 mm, SPL ~92 dB, 120 Hz–12 kHz. | MicroJPM | **3.95** |

---

## 5) Alimentación y Regulación

| **Componente** | **Descripción / Función** | **Especificaciones clave** | **Proveedor / Enlace** | **Costo (USD)** |
|---|---|---|---|---:|
| **Li-Ion Polymer 3.7 V / 2000 mAh** | Batería principal recargable del sistema. | 3.7 V nominal, protección integrada. | CRCibernética | **14.95** |
| **TP4056 1A Charger Module (Mini-B)** | Cargador Li-Ion 1 A sin protección (carga/descarga). | Entrada Mini-B| Microjpm | **4.50** |
| **MT3608 DC-DC Boost** | Eleva 3.7 V → 5 V (servos + DFPlayer). | 2–24 V in, ~5–28 V out, hasta 1 A recomendado. | CRCibernética | **2.49** |
| **LM2596 DC-DC Buck** | Reduce 5 V → 3.3 V (ESP8266 + HX711). | 4.5–40 V in, 1.25–35 V out, 3 A máx. | CRCibernética | **4.95** |

---

## 6) Comunicación / Detección de presencia

| **Componente** | **Descripción / Función** | **Especificaciones clave** | **Costo (USD)** |
|---|---|---|---:|
| **Sensor IR de presencia (TCRT5000)** | Señal digital para “moneda detectada” antes del pesaje. | Detección de objetos cercanos (0.2 a 2.5 mm) | **1.60** |

---

## 7) 3D e Insumos (variables)

| **Concepto** | **Descripción / Observación** | **Costo** |
|---|---|---:|
| **Impresión 3D del cuerpo (Gimmighoul + guías)** | Laimi: **₡50 por gramo**. El costo final depende del peso total impreso. | **₡ 50 × (gramos)** |
| **Consumibles de conexión** | Cables, headers, tornillería ligera. | — |

---

## 8) Resumen de Costos (según anteproyecto)

| **Categoría** | **Subtotal (USD)** |
|---|---:|
| Sensores (TAL221 + HX711) | **16.95 + 8.95 = 25.90** |
| Actuadores (2× SG90) | **11.90** |
| Audio (DFPlayer + parlante) | **4.95 + 3.95 = 8.90** |
| Alimentación y regulación (batería + TP4056 + MT3608 + LM2596) | **14.95 + 4.95 + 2.49 + 4.95 = 27.34** |
| Microcontrolador (Mini D1 Wemos ESP8266) | **0.00** |
| **Total Aproximado (sin impresión 3D)** | **$74.04** |

**Total en colones (ref. ₡502.01 / USD):** **₡37 168.82**  
**Impresión 3D (variable):** **₡50 × (gramos)** → *se suma al total en CRC cuando se defina el peso final.*

---

## 9) Observaciones técnicas

- **Comunicación digital principal:** HX711 (2-wire: DT/SCK) entre celda de carga y ESP8266.  
- **Comunicación secundaria:** UART entre ESP8266 y DFPlayer Mini para audio.  
- **Líneas de alimentación recomendadas:**  
  - Batería 3.7 V → **TP4056** (carga).  
  - Batería 3.7 V → **MT3608** (boost a 5 V: servos + DFPlayer).  
  - **LM2596** (buck de 5 V a **3.3 V** para ESP8266 + HX711).  
- Añadir **desacoplos** en potencia: ≥470–1000 µF en 5 V (picos de los SG90) y 100–220 µF en 3.3 V.  
- **GND común** para todos los módulos.  
- El **sensor IR** solo triggea el flujo; la **validación de moneda** se realiza por **peso** (mayor confiabilidad).
