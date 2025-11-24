# Bill of Materials (BOM) – Proyecto GimmiCoin  

---

## 1. Introducción

El siguiente documento detalla la **lista de materiales** utilizados en la construcción del robot ―alcancía inteligente― **GimmiCoin**, correspondiente al diseño actualizado del sistema:

- Microcontrolador ESP8266 (D1 Mini)
- Sensor de contacto
- Celda de carga TAL221 + HX711
- Motor DC + etapa de potencia 2N2222 + diodo flyback
- DFPlayer Mini + parlante
- Webserver embebido
- Powerbank 5V
- Montaje final en **tarjeta perforada**

Todos los precios están actualizados a colones costarricenses (CRC).

---

## 2. Lista de materiales completa

| # | Componente | Cantidad | Precio unitario (USD) | Precio unitario (CRC) | Proveedor | Notas |
|---|------------|----------|------------------------|------------------------|-----------|--------|
| 1 | D1 Mini ESP8266 | 1 | $3.90 | ₡2 000 | AliExpress | Control principal |
| 2 | Celda de carga TAL221 (100g) | 1 | $3.50 | ₡1 800 | AliExpress | Sensor de pesaje |
| 3 | Módulo HX711 | 1 | $1.20 | ₡650 | AliExpress | ADC 24 bits para balanza |
| 4 | Motor DC 3–6V | 1 | $1.00 | ₡520 | Electrónica Kapro | Actuador para limpieza |
| 5 | Transistor 2N2222 | 1 | $0.10 | ₡50 | Electrónica Kapro | Etapa de potencia |
| 6 | Diodo flyback 1N4148 o 1N4007 | 1 | $0.05 | ₡30 | Electrónica Kapro | Protección del motor |
| 7 | Resistencia 10 kΩ (sensor contacto) | 1 | $0.05 | ₡30 | Electrónica Kapro | Pull-down |
| 8 | Resistencia 1 kΩ (DFPlayer RX) | 1 | $0.05 | ₡30 | Electrónica Kapro | Protección UART |
| 9 | DFPlayer Mini | 1 | $2.50 | ₡1 300 | AliExpress | Sonido del Gimmighoul |
| 10 | Parlante 8Ω 0.5W | 1 | $1.00 | ₡520 | AliExpress | Salida de audio |
| 11 | Powerbank 5V 2000–5000 mAh | 1 | $5.00 | ₡2 600 | Walmart / Electrónica | Fuente de energía |
| 12 | Tarjeta perforada | 1 | $1.00 | ₡520 | Electrónica Kapro | Montaje final obligatorio |
| 13 | Cables Dupont / cable sólido | 20 | $0.05 | ₡30 c/u | Electrónica Kapro | Conexiones |
| 14 | Protuberancias / separadores plásticos | 4 | $0.10 | ₡50 | Electrónica Kapro | Aislamiento mecánico |
| 15 | Tornillería M3 / tuercas | 6 | $0.10 | ₡50 | Ferretería | Fijación de celda/motor |
| 16 | Cinta doble adhesiva 3M | 1 | $1.00 | ₡520 | Oficentro | Montaje mecánico |
| 17 | Caja / estructura Gimmighoul (impresión 3D o manual) | 1 | $4.00 | ₡2 000 | Local / impresión | Envoltura física |

---

## 3. Costos totales

### 3.1 Total en USD  
Suma de todos los componentes:

**≈ $29.00**

### 3.2 Total en CRC  
Conversión usando **₡520 por dólar**:

**≈ ₡15 080**

(El valor puede variar levemente según proveedor y stock.)

---

## 4. Notas importantes

1. **No se usan servos ni sensores IR**, por lo que fueron removidos del BOM original.  
2. El sistema utiliza **solo un motor DC**, mediante un transistor en modo conmutación.  
3. La alimentación se simplifica notablemente con el uso de una **powerbank**, eliminando cargadores LiPo, boost, buck y baterías Li-ion.  
4. El BOM incluye componentes indispensables no considerados originalmente:  
   - resistencias  
   - diodo flyback  
   - tarjeta perforada  
5. La estructura física puede imprimirse en 3D o construirse manualmente.  

---
