// Duración del pulso del motor en milisegundos
const int T_MOTOR = 750;

// Inicialización del motor
void motor_init() {
  pinMode(PIN_MOTOR, OUTPUT);
  digitalWrite(PIN_MOTOR, LOW);  // motor apagado
}

// Activar el motor por un tiempo definido
void motor_pulse() {
  digitalWrite(PIN_MOTOR, HIGH);
  delay(T_MOTOR);
  digitalWrite(PIN_MOTOR, LOW);
}
