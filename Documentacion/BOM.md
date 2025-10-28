# Bill of Materials (BOM)

**Proyecto:** Gimmighoul Coin Collector Robot  
**Curso:** CE4301 - Arquitectura de Computadores I  
**Instituto Tecnológico de Costa Rica**

---

## 1. Microcontrolador y Programación

| **Componente** | **Descripción / Función** | **Especificaciones Clave** | **Proveedor / Enlace** | **Costo (USD)** |
|----------------|---------------------------|-----------------------------|-------------------------|------------------|
| **ESP32-S WiFi + Bluetooth Module** | Microcontrolador principal del sistema, encargado del procesamiento, lectura de sensores y control de actuadores. | Wi-Fi + Bluetooth 4.2, Dual-Core 32-bit, 240 MHz, 3.3 V lógico, soporta UART / I2C / SPI. | [MicroJPM](https://www.microjpm.com/products/ad75611) | 11.95 |
| **FT232RL USB → UART Converter** | Conversor USB a TTL para programar el ESP32-S mediante conexión serial. | Compatible con 3.3 V y 5 V, USB micro. | [MicroJPM](https://www.microjpm.com/products/ad36517) | 8.50 |

---

## 2. Sensores

| **Componente** | **Descripción / Función** | **Especificaciones Clave** | **Proveedor / Enlace** | **Costo (USD)** |
|----------------|---------------------------|-----------------------------|-------------------------|------------------|
| **Load Cell TAL221 (100 g Mini Straight Bar)** | Sensor de peso para detección y clasificación de monedas según masa. | Capacidad 100 g, aluminio, 4 galgas, salida diferencial. | [MicroJPM](https://www.microjpm.com/products/ad39155) | 16.95 |
| **HX711 Load Cell Amplifier** | Módulo amplificador y convertidor ADC de 24 bits para la celda de carga. | Interfaz digital tipo SPI (2 hilos: DT, SCK), 2.7–5 V, bajo ruido. | [CRCibernética](https://www.crcibernetica.com/load-cell-amplifier-hx711/) | 8.95 |

---

## 3. Actuadores

| **Componente** | **Descripción / Función** | **Especificaciones Clave** | **Proveedor / Enlace** | **Costo (USD)** |
|----------------|---------------------------|-----------------------------|-------------------------|------------------|
| **Servo Motor SG90 (x2)** | Servomotores para: (1) apertura de la tapa del cofre y (2) expulsión o limpieza de la moneda tras medición. | 5 V, 180°, torque 1.8 kg·cm, consumo máx. 700 mA. | [MicroJPM](https://www.microjpm.com/products/mini-servo-towerpro-sg90-con-accesorios/) | 5.95 × 2 |

---

## 4. Audio y Comunicación Digital Secundaria

| **Componente** | **Descripción / Función** | **Especificaciones Clave** | **Proveedor / Enlace** | **Costo (USD)** |
|----------------|---------------------------|-----------------------------|-------------------------|------------------|
| **DFPlayer Mini MP3 Module** | Módulo reproductor de audio para emitir sonidos del Pokémon al detectar monedas. | UART 9600 bps, DAC 24 bits, salida directa SPK (8 Ω / 0.5 W), soporte FAT16/32. | [CRCibernética](https://www.crcibernetica.com/mini-mp3-player-module/) | 4.95 |
| **Parlante 8 Ω / 0.5 W** | Reproducción directa del DFPlayer (efecto de sonido). | 57 mm diámetro, SPL 92 dB, rango 120 Hz–12 kHz. | [MicroJPM](https://www.microjpm.com/products/ad29923/) | 3.95 |

---

## 5. Alimentación y Regulación de Energía

| **Componente** | **Descripción / Función** | **Especificaciones Clave** | **Proveedor / Enlace** | **Costo (USD)** |
|----------------|---------------------------|-----------------------------|-------------------------|------------------|
| **Polymer Li-Ion Battery 3.7 V / 2000 mAh** | Fuente principal recargable del sistema. | 3.7 V nominal, 2000 mAh, protección integrada contra sobrecarga y cortocircuito. | [CRCibernética](https://www.crcibernetica.com/polymer-lithium-ion-battery-3-7v-2000mah/) | 14.95 |
| **TP4056 Li-Ion Charger (USB-C, Dual Protection)** | Cargador para baterías de ion de litio, entrada USB-C. | Corriente de carga 1 A, protección por sobrecarga y sobredescarga. | [CRCibernética](https://www.crcibernetica.com/tp4056-lithium-battery-charger-module-with-dual-protection-usb-c/) | 4.95 |
| **MT3608 DC–DC Boost Converter** | Eleva 3.7 V de la batería a 5 V para servos y DFPlayer. | Entrada 2–24 V, salida ajustable hasta 28 V, 2 A máx. | [CRCibernética](https://www.crcibernetica.com/mt3608-2a-2v-24v-dc-dc-booster-power-module/) | 2.49 |
| **LM2596 DC–DC Buck Converter** | Reduce los 5 V del booster a 3.3 V estables para el ESP32 y sensores. | 4.5–40 V entrada, 1.25–35 V salida, 3 A máx. | [CRCibernetica](https://www.crcibernetica.com/lm2596-dc-dc-buck-converter-step-down-power-module-output-1-25v-35v/) | 4.95 |

---

## 6. Componentes Auxiliares y Montaje

| **Componente** | **Descripción / Función** | **Especificaciones Clave** | **Costo Estimado (USD)** |
|----------------|---------------------------|-----------------------------|----------------------------|
| Interruptor ON/OFF | Control general de encendido del sistema (en la línea OUT del TP4056). | Corriente nominal ≥ 2 A, compatible 3.7 V. | Por definir |
| Cables Dupont / Conectores / Headers | Conexiones entre módulos. | Macho–hembra / macho–macho. | 2.00 |
| Tornillería y Separadores | Montaje mecánico de la celda de carga, servos y estructura. | M2–M3, separadores 10–15 mm. | Por definir |

---

## 7. Resumen de Costos

| **Categoría** | **Subtotal (USD)** |
|----------------|--------------------|
| Microcontrolador y programación | Por definir |
| Sensores (celda + amplificador) | Por definir |
| Actuadores (2 servos) | Por definir |
| Audio y comunicación | Por definir |
| Alimentación y regulación | Por definir |
| Auxiliares y montaje | Por definir |
| **Total Aproximado** | **Por definir** |

---

## 8. Observaciones Técnicas

- El sistema funciona completamente autónomo con alimentación por batería.  
- La celda de carga y el HX711 constituyen la **comunicación digital principal** entre el sensor y el ESP32-S.  
- El DFPlayer Mini usa **UART** como **comunicación digital secundaria** con el ESP32-S.  
- Todos los módulos son **compatibles a nivel lógico** (3.3 V) y **comparten GND común**.  
- Se recomienda la adición de **capacitores de desacoplo** (470–1000 µF en 5 V, 100–220 µF en 3.3 V) para evitar caídas de tensión durante picos de corriente.

---
