// =====================================================================================
// Módulo del motor DC - Control básico de activación por pulsos
// =====================================================================================

static bool motorActivo = false;
static unsigned long motorInicioMs = 0;
const unsigned long MOTOR_DURACION_PULSO_MS = 500;  // duración nominal del impulso

void motor_init() {
  pinMode(PIN_MOTOR, OUTPUT);
  digitalWrite(PIN_MOTOR, LOW);
  Serial.println(F("[MOTOR] Motor DC inicializado."));
}

void motor_pulse() {
  motor_start();
}

void motor_start() {
  digitalWrite(PIN_MOTOR, HIGH);
  motorActivo = true;
  motorInicioMs = millis();
  Serial.println(F("[MOTOR] Motor activado."));
}

void motor_stop() {
  digitalWrite(PIN_MOTOR, LOW);
  motorActivo = false;
  Serial.println(F("[MOTOR] Motor desactivado."));
}

bool motor_isActive() {
  if (motorActivo && (millis() - motorInicioMs >= MOTOR_DURACION_PULSO_MS)) {
    motor_stop();
  }
  return motorActivo;
}

unsigned long motor_pulseDurationMs() {
  return MOTOR_DURACION_PULSO_MS;
}
