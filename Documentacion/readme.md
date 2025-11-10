## Descripción de Módulos Principales

### Main.ino
- **Función principal:** Configura el sistema y controla el flujo general del programa.
- **Tareas principales:**
  - Inicializa el WiFi y el servidor web.
  - Configura el pin D2 (GPIO4) como entrada con INPUT_PULLUP.
  - Carga el contador guardado desde la EEPROM.
  - Asocia una interrupción al pin D2.
  - En el loop(), atiende el servidor web y guarda el contador cada 10 pulsos.

- **Funciones principales:**
  - setup(): Inicializa componentes y recupera datos de la EEPROM.
  - loop(): Ejecuta tareas continuas y verifica cuándo guardar en memoria.
  - Write_Value_Mem(Direc, Value): Escribe un valor en la EEPROM y lo confirma con commit().

---

### Interrupt.ino
- **Función principal:** Gestiona la interrupción generada por el pin D2 cuando se detecta una moneda (contacto con GND).
- **Funciones principales:**
  - Contar_Pulsos():
    - Se ejecuta cuando el pin D2 detecta una señal baja.
    - Aplica un filtro de rebote (debouncer) de 120 ms.
    - Incrementa el contador global (Counter) y el contador auxiliar (MemwriteLimit).

---

### Mem_manage.ino
- **Función principal:** Archivo reservado para futuras funciones de manejo de memoria.
- **Funciones principales:** (actualmente vacío) 

## 📂 Descripción de Módulos Secundarios


## calibracion.ino
### Descripción general
Este módulo se encarga del proceso de calibración de sensores, asegurando que las lecturas estén correctamente ajustadas antes del uso.

### Funciones principales
- `setup()`: Describe la función de `setup`.
- `loop()`: Describe la función de `loop`.


## deteccion_monedas.ino
### Descripción general
Este módulo detecta monedas mediante sensores o señales, identificando su presencia y posiblemente su valor.

### Funciones principales
- `promedioEstable()`: Describe la función de `promedioEstable`.
- `setup()`: Describe la función de `setup`.
- `loop()`: Describe la función de `loop`.


## Mem_reset.ino
### Descripción general
Este módulo gestiona la memoria del sistema, permitiendo reiniciar o limpiar datos almacenados para volver al estado inicial.

### Funciones principales
- `setup()`: Describe la función de `setup`.
- `loop()`: Describe la función de `loop`.
