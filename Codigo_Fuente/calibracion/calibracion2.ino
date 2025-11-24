// =======================================================
//  Parte 1: Compatibilidad
// =======================================================
#if !defined(__XTENSA__)
  #undef ARDUINO
  #undef GM_USING_ARDUINO_CORE
  #define GM_USING_ARDUINO_CORE 0
#endif
#include "Arduino.h"

// =======================================================
//  Parte 2: HX711
// =======================================================
#include <HX711.h>

// Ajusta estos pines a tu conexión real del HX711
const int LOADCELL_DOUT_PIN = D5;   // DT del HX711
const int LOADCELL_SCK_PIN  = D6;   // SCK del HX711

HX711 scale;

#define PIN_SENSOR_MONEDA   D2     // Sensor de moneda (pull-down externo)

// =======================================================
//  Parte 3: Sensor de moneda
// =======================================================

// Tiempo mínimo entre pulsos (en microsegundos) para evitar rebotes
// 300000 us = 300 ms
const unsigned long SENSOR_DEBOUNCE_US = 300000;

// Variables compartidas con la ISR
volatile bool          sensorMoneda_flagNuevaMoneda = false;
volatile uint32_t      sensorMoneda_contador = 0;
volatile unsigned long sensorMoneda_ultimoPulso = 0;

// NUEVO: bloqueo para que la ISR solo acepte un pulso por moneda
volatile bool          sensorMoneda_bloqueado = false;

// Umbral para considerar que NO hay moneda en la celda
// Como estás con set_scale(1.0), sin moneda estarás cerca de 0,
// y con moneda en ~ |140000|. Ajusta si hace falta.
const float UMBRAL_SIN_MONEDA_UNITS = 10000.0f;

// Estado en el loop: estamos esperando que retiren la moneda
bool estadoEsperandoRetiro = false;

// =======================================================
//  Parte 3.5: Clasificación de monedas por rangos
// =======================================================

// Rangos "seguros" basados en datos experimentales (valores UNITS con set_scale())
// Solo aceptamos monedas que caigan en estos rangos sin solapamiento

// 100 colones: rango muy negativo, bien separado
const float RANGO_100_MIN = -115000.0f;
const float RANGO_100_MAX = -85000.0f;

// 25 colones: rango negativo medio-alto  
const float RANGO_25_MIN = -70000.0f;
const float RANGO_25_MAX = -55000.0f;

// 50 colones: rango negativo medio-bajo
const float RANGO_50_MIN = -48000.0f;
const float RANGO_50_MAX = -35000.0f;

// 10 colones normales: rango positivo bajo
const float RANGO_10_MIN = -2000.0f;
const float RANGO_10_MAX = 12000.0f;

// 10 colones aluminio: rango positivo medio
const float RANGO_10A_MIN = 14000.0f;
const float RANGO_10A_MAX = 25000.0f;

/* Function: clasificarMonedaPorRangos
   Clasifica una moneda basándose en el valor UNITS de la balanza HX711.
   
   Parámetros:
   - units: valor obtenido de scale.get_units() 
   
   Retorna:
   - 100: moneda de 100 colones
   - 25:  moneda de 25 colones  
   - 50:  moneda de 50 colones
   - 10:  moneda de 10 colones normal
   - 11:  moneda de 10 colones aluminio (para diferenciar)
   - 0:   desconocido/no clasificable (fuera de rangos seguros)
*/
uint8_t clasificarMonedaPorRangos(float units) {
  // Verificar cada rango en orden de magnitud
  
  if (units >= RANGO_100_MIN && units <= RANGO_100_MAX) {
    return 100;  // 100 colones
  }
  
  if (units >= RANGO_25_MIN && units <= RANGO_25_MAX) {
    return 25;   // 25 colones
  }
  
  if (units >= RANGO_50_MIN && units <= RANGO_50_MAX) {
    return 50;   // 50 colones
  }
  
  if (units >= RANGO_10_MIN && units <= RANGO_10_MAX) {
    return 10;   // 10 colones normal
  }
  
  if (units >= RANGO_10A_MIN && units <= RANGO_10A_MAX) {
    return 11;   // 10 colones aluminio
  }
  
  return 0;      // No clasificable / desconocido
}

/* Function: obtenerNombreMoneda
   Convierte el código numérico de moneda a texto descriptivo.
*/
const char* obtenerNombreMoneda(uint8_t codigo) {
  switch(codigo) {
    case 100: return "100 colones";
    case 50:  return "50 colones";
    case 25:  return "25 colones"; 
    case 10:  return "10 colones";
    case 11:  return "10 colones (aluminio)";
    case 0:   return "DESCONOCIDA";
    default:  return "ERROR";
  }
}

/* Function: sensorMoneda_isr
   Atiende la interrupción del sensor de moneda aplicando antirrebote.
*/
void IRAM_ATTR sensorMoneda_isr() {
  // Si está bloqueado, no aceptar más pulsos
  if (sensorMoneda_bloqueado) {
    return;
  }

  unsigned long ahora = micros();

  // Debounce por tiempo
  if (ahora - sensorMoneda_ultimoPulso > SENSOR_DEBOUNCE_US) {
    sensorMoneda_ultimoPulso = ahora;
    sensorMoneda_flagNuevaMoneda = true;
    sensorMoneda_contador++;

    // IMPORTANTE: bloquear hasta que el loop decida rearmar
    sensorMoneda_bloqueado = true;
  }
}


/* Function: sensorMoneda_init
   Configura el pin del sensor y registra la interrupción correspondiente.
*/
void sensorMoneda_init() {
  pinMode(PIN_SENSOR_MONEDA, INPUT);   

  // Interrupción por flanco ascendente
  attachInterrupt(digitalPinToInterrupt(PIN_SENSOR_MONEDA),
                  sensorMoneda_isr,
                  RISING);

  Serial.println(F("[SensorMoneda] Inicializado (pull-down externo, interrupción RISING)"));
}


/* Function: sensorMoneda_hayNuevaMoneda
   Consulta si la ISR ha registrado una nueva moneda desde la última lectura.
*/
bool sensorMoneda_hayNuevaMoneda() {
  noInterrupts();
  bool hay = sensorMoneda_flagNuevaMoneda;
  if (hay) {
    sensorMoneda_flagNuevaMoneda = false;
  }
  interrupts();
  return hay;
}


/* Function: sensorMoneda_getConteo
   Devuelve el total acumulado de monedas detectadas por la ISR.
*/
uint32_t sensorMoneda_getConteo() {
  noInterrupts();
  uint32_t c = sensorMoneda_contador;
  interrupts();
  return c;
}


/* Function: sensorMoneda_setConteo
   Sincroniza el contador de la ISR con un valor proporcionado externamente.
*/
void sensorMoneda_setConteo(uint32_t value) {
  noInterrupts();
  sensorMoneda_contador = value;
  sensorMoneda_flagNuevaMoneda = false;
  sensorMoneda_ultimoPulso = micros();
  interrupts();
}

/* NUEVO: rearmar el sensor de moneda (permitir un nuevo pulso) */
void sensorMoneda_rearmar() {
  noInterrupts();
  sensorMoneda_bloqueado = false;
  sensorMoneda_flagNuevaMoneda = false;
  interrupts();
  Serial.println(F("[SensorMoneda] Rearmado (lista para nueva moneda)"));
}

/* Function: realizarTareBalanza
   Realiza un tare de la balanza con múltiples mediciones para mayor precisión.
   Se debe llamar solo cuando la balanza está completamente vacía.
*/
void realizarTareBalanza() {
  Serial.println(F("[HX711] Realizando tare (balanza vacía)..."));
  
  // Pequeña pausa para estabilización
  delay(1000);
  
  // Hacer tare con múltiples lecturas para mayor precisión
  scale.tare(25);   // promedio de 25 lecturas para el nuevo cero
  
  Serial.println(F("[HX711] Tare completado - nuevo cero establecido"));
}

// =======================================================
//  Parte 4: setup() y loop() para leer HX711 tras el flanco
// =======================================================

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println(F("\n=== Inicializando HX711 + Sensor de Moneda ==="));
  Serial.println(F("Sistema con clasificación automática por rangos"));
  Serial.println(F("Auto-tare: al inicio y después de cada moneda"));
  Serial.println(F("Monedas soportadas: 100, 50, 25, 10 y 10 (aluminio) colones"));

  // 1) Inicializar HX711
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // Para calibrar: usamos factor 1.0 (no está en gramos aún)
  scale.set_scale();

  // Tara inicial mejorada usando la función dedicada
  Serial.println(F("Asegúrese de que no hay nada sobre la balanza"));
  delay(2000);  // Dar tiempo para estabilizar
  realizarTareBalanza();

  // 2) Inicializar sensor de moneda (interrupción en D2 / PIN_SENSOR_MONEDA)
  sensorMoneda_init();

  // 3) Mostrar rangos de clasificación para referencia
  Serial.println(F("\n--- Rangos de clasificación (UNITS) ---"));
  Serial.println(F("100 colones:      -115000 a -85000"));
  Serial.println(F("25 colones:       -70000 a -55000"));  
  Serial.println(F("50 colones:       -48000 a -35000"));
  Serial.println(F("10 colones:       -2000 a 12000"));
  Serial.println(F("10 col (aluminio): 14000 a 25000"));
  Serial.println(F("Fuera de rango:   DESCONOCIDA"));
  Serial.println(F("---------------------------------------"));

  Serial.println(F("[Sistema] Listo. Inserta una moneda..."));
}

void loop() {
  // 1) Revisar si la ISR reportó un nuevo flanco (nueva moneda)
  if (sensorMoneda_hayNuevaMoneda()) {
    uint32_t conteo = sensorMoneda_getConteo();

    Serial.println();
    Serial.print(F("[Evento] Nueva moneda detectada. Conteo total = "));
    Serial.println(conteo);

    // Ya no queremos aceptar más pulsos por esta misma moneda
    estadoEsperandoRetiro = true;

    // 2) Esperar 6 segundos para que la moneda se estabilice
    Serial.println(F("[Evento] Esperando 6 segundos para estabilizar..."));
    delay(6000);

    // 3) Leer HX711 (promedio de N lecturas)
    const uint8_t N = 10;   // número de lecturas para promediar

    long  raw   = scale.read_average(N);      // valor crudo promedio
    float units = scale.get_units(N);         // valor con escala actual (1.0 -> unidades "brutas - tare")

    // 4) Clasificar la moneda y mostrar resultados
    Serial.println(F("----- MEDICION HX711 -----"));
    Serial.print(F("Lecturas promediadas: "));
    Serial.println(N);

    Serial.print(F("RAW promedio (read_average): "));
    Serial.println(raw);

    Serial.print(F("UNITS promedio (get_units con scale=1.0): "));
    Serial.println(units, 3);

    // NUEVA FUNCIONALIDAD: Clasificar moneda por rangos
    uint8_t tipoMoneda = clasificarMonedaPorRangos(units);
    const char* nombreMoneda = obtenerNombreMoneda(tipoMoneda);
    
    Serial.println(F(""));
    Serial.println(F("----- CLASIFICACION -----"));
    Serial.print(F("Moneda detectada: "));
    Serial.print(nombreMoneda);
    if (tipoMoneda > 0) {
      Serial.print(F(" (código: "));
      Serial.print(tipoMoneda);
      Serial.println(F(")"));
    } else {
      Serial.println(F(" - Valor fuera de rangos seguros"));
      Serial.println(F("Posibles causas:"));
      Serial.println(F("- Moneda no soportada"));
      Serial.println(F("- Lectura con ruido"));
      Serial.println(F("- Moneda mal posicionada"));
    }

    Serial.println(F("---------------------------"));
    Serial.println(F("[Sistema] Esperando que retiren la moneda..."));
  }

  // 2) Si estamos esperando que retiren la moneda, revisamos el peso
  if (estadoEsperandoRetiro) {
    // Lectura rápida para ver si ya no hay peso
    float unitsAhora = scale.get_units(3);

    // También podemos comprobar el nivel del sensor de moneda, opcional:
    int nivelSensor = digitalRead(PIN_SENSOR_MONEDA);

    // Condición para considerar que la moneda fue retirada:
    // - Peso cercano a cero
    // - Y (opcional) sensor en nivel bajo
    if (fabs(unitsAhora) < UMBRAL_SIN_MONEDA_UNITS && nivelSensor == LOW) {
      Serial.println(F("[Sistema] Moneda retirada, rearmando sensor..."));
      
      // NUEVO: Hacer tare automático después de retirar la moneda
      Serial.println(F("[Sistema] Recalibrando cero de referencia..."));
      realizarTareBalanza();
      
      sensorMoneda_rearmar();
      estadoEsperandoRetiro = false;

      Serial.println(F("[Sistema] Puedes insertar otra moneda."));
    }
  }

  // Aquí puedes hacer otras tareas si quieres
}
