# Arquitectura del Software Embebido

## Diagrama de Flujo del Programa Principal

El flujo principal del programa se puede resumir en el siguiente diagrama:

1. **Inicio**:
   - Configuración de pines y periféricos.
   - Inicialización del servidor web y EEPROM.
   - Recuperación del contador almacenado en memoria.

2. **Bucle Principal**:
   - El programa opera en un bucle infinito (`loop`) donde se gestionan las solicitudes HTTP y se actualiza el estado del sistema.

3. **Interrupciones**:
   - Las interrupciones gestionan la detección de monedas mediante un sensor de conductividad.

```plaintext
Inicio → Configuración → Bucle Principal → Gestión de Interrupciones
```

## Estrategias de Gestión de E/S

### Manejo de Interrupciones

- **Interrupción por cambio de estado**:
  - El pin conectado al sensor de monedas genera una interrupción en cada cambio de estado (HIGH → LOW o LOW → HIGH).
  - Se utiliza un debounce temporal para evitar conteos múltiples no deseados.

```cpp
void IRAM_ATTR Contar_Moneda() {
  bool currentState = digitalRead(BUTTON_PIN);
  if (currentState == LOW && lastState == HIGH) {
    unsigned long now = micros();
    if (now - lastValidTime > minPulseInterval) {
      Counter++;
      MemwriteLimit++;
      lastValidTime = now;
    }
  }
  lastState = currentState;
}
```

### Prioridades y Sincronización

- **Prioridad**:
  - Las interrupciones tienen prioridad sobre el bucle principal.
  - La escritura en EEPROM se realiza de forma diferida para evitar bloqueos.

- **Sincronización**:
  - Variables compartidas (`Counter`, `MemwriteLimit`) se declaran como `volatile` para garantizar consistencia entre ISR y el programa principal.

## Protocolos de Comunicación

### HTTP (Servidor Web)

- **Protocolo Implementado**:
  - El servidor web embebido utiliza HTTP para exponer endpoints como `/` (estado general) y `/reset` (reinicio del contador).

- **Análisis Comparativo**:
  - HTTP fue elegido por su simplicidad y compatibilidad con navegadores web.
  - Alternativas como MQTT o WebSocket no se implementaron debido a la ausencia de un backend externo o requisitos de comunicación en tiempo real.

### Ejemplo de Endpoint

```cpp
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Alcancía Inteligente</title>
</head>
<body>
<h1>Total de monedas: " + String(Counter) + "</h1>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}
```

## Organización de Memoria

### EEPROM

- **Uso**:
  - Se utiliza para almacenar el contador de monedas de forma persistente.
  - Dirección `0` reservada para el valor del contador.

- **Optimización**:
  - La escritura en EEPROM se realiza solo después de un número definido de eventos (`MemwriteLimit`) para minimizar el desgaste.

### Variables Globales

- **Volátiles**:
  - `Counter`, `MemwriteLimit`, `lastState` y `lastValidTime` se declaran como `volatile` para garantizar consistencia en entornos multitarea.

- **Constantes**:
  - `minPulseInterval` define el intervalo mínimo entre pulsos para evitar rebotes.

---

Esta arquitectura asegura un diseño eficiente y robusto, optimizando el uso de recursos y garantizando la persistencia de datos críticos.