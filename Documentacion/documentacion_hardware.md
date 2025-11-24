# Diseño del Robot: GimmiCoin – Alcancía Inteligente

## Selección y Justificación del Microcontrolador

El microcontrolador seleccionado para este proyecto es el Mini D1 Wemos ESP8266. Este dispositivo fue elegido debido a las siguientes características clave:

- **Conectividad Wi-Fi integrada**: Permite la implementación de un servidor web embebido para la interfaz de usuario.
- **Tamaño compacto**: Ideal para integrarse en el diseño físico de la alcancía.
- **Amplia comunidad de soporte**: Facilita el desarrollo y resolución de problemas.
- **Capacidad de procesamiento adecuada**: Suficiente para manejar múltiples sensores, actuadores y la lógica del sistema.

## Diagramas de Conexión

A continuación, se describe la conexión de los principales componentes del sistema:

1. **Celda de carga y módulo HX711**:
   - La celda de carga TAL221 (100 g) está conectada al módulo HX711, que amplifica y convierte las señales analógicas a digitales.
   - Conexión del HX711 al ESP8266:
     - DT → Pin D2
     - SCK → Pin D3
     - VCC → 3.3V
     - GND → GND

2. **Módulo DFPlayer Mini**:
   - Utilizado para la reproducción de audio.
   - Conexión:
     - RX → Pin D5 (a través de un divisor de voltaje para evitar daño por niveles lógicos).
     - TX → Pin D6
     - VCC → 5V
     - GND → GND

3. **Motor DC**:
   - Controlado mediante un puente H o un driver de motor (L298N o similar).
   - Conexión:
     - IN1, IN2 → Pines digitales del ESP8266 (D7, D8).
     - VCC → Fuente externa de 5V.
     - GND → GND común.

4. **Sensor de conductividad**:
   - Detecta la inserción de monedas mediante un puente de conductores.
   - Conexión:
     - Señal → Pin D1
     - VCC → 3.3V
     - GND → GND

## Especificaciones de Sensores y Actuadores

- **Celda de carga TAL221**: Capacidad máxima de 100 g, precisión de 0.01 g.
- **Módulo HX711**: ADC de 24 bits para lecturas precisas.
- **DFPlayer Mini**: Reproducción de archivos MP3 desde una tarjeta microSD.
- **Motor DC**: Velocidad y torque adecuados para mover monedas al interior.
- **Sensor de conductividad**: Detecta flancos de señal al insertar monedas.

## Diseño del PCB/Tarjeta Perforada

El diseño del circuito se realizó en una tarjeta perforada para prototipos, con las siguientes consideraciones:

- **Distribución compacta**: Minimiza el espacio ocupado dentro de la estructura.
- **Rutas de alimentación separadas**: Evita interferencias entre componentes sensibles como el HX711 y actuadores como el motor DC.
- **Conectores modulares**: Facilita el mantenimiento y reemplazo de componentes.

## Análisis de Consumo Energético

El sistema está diseñado para ser alimentado por un powerbank de 10,000 mAh con salida de 5V y 3A. A continuación, se presenta un análisis estimado del consumo:

| Componente         | Consumo (mA) |
|--------------------|--------------|
| ESP8266           | 70-200       |
| HX711 + Celda     | 10           |
| DFPlayer Mini     | 40-100       |
| Motor DC          | 150-300      |
| Total estimado    | 270-610      |

Con un consumo promedio de 500 mA, el sistema puede operar aproximadamente 20 horas continuas.

## Consideraciones de Diseño Mecánico

- **Estructura**: Construida con materiales reciclados (cartón, plástico y madera) para reducir costos y fomentar la sostenibilidad.
- **Compartimentos internos**: Diseñados para separar la electrónica de las monedas y evitar daños.
- **Acceso para mantenimiento**: La tapa superior es desmontable para facilitar el acceso a los componentes internos.
- **Estabilidad**: Base amplia para evitar vuelcos durante el uso.

---

Este diseño combina eficiencia, funcionalidad y sostenibilidad, cumpliendo con los objetivos académicos y prácticos del proyecto.