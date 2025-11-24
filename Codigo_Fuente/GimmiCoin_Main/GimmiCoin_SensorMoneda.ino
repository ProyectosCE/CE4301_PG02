// =====================================================================================
// Módulo del sensor de moneda - Lectura sencilla con conteo en RAM
// =====================================================================================

static uint32_t sensorConteo = 0;

void sensorMoneda_init() {
  pinMode(PIN_SENSOR_MONEDA, INPUT);
  Serial.println(F("[SENSOR] Sensor de moneda inicializado."));
}

bool sensorMoneda_hayNuevaMoneda() {
  return digitalRead(PIN_SENSOR_MONEDA) == HIGH;
}

uint32_t sensorMoneda_getConteo() {
  return sensorConteo;
}

void sensorMoneda_setConteo(uint32_t value) {
  sensorConteo = value;
}