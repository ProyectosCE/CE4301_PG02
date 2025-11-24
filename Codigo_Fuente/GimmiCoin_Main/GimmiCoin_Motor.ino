// Duración del pulso del motor en milisegundos
const int T_MOTOR = 2000;

static bool motorActive = false;

// Inicialización del motor
/* Function: motor_init
   Configura el pin de control del motor y garantiza su estado apagado.

   Params:
     - Ninguno.

   Returns:
     - void - no retorna valor.
*/
void motor_init() {
  pinMode(PIN_MOTOR, OUTPUT);
  digitalWrite(PIN_MOTOR, LOW);  // motor apagado
  motorActive = false;
}

// Activar el motor por un tiempo definido
/* Function: motor_pulse
   Activa el motor por un intervalo fijo para transportar la moneda.

   Params:
     - Ninguno.

   Returns:
     - void - no retorna valor.

   Restriction:
     La activación supone que la fuente de alimentación puede suministrar la corriente requerida.
*/
void motor_pulse() {
  motor_start();
  delay(T_MOTOR);
  motor_stop();
}

void motor_start() {
  digitalWrite(PIN_MOTOR, HIGH);
  motorActive = true;
}

void motor_stop() {
  digitalWrite(PIN_MOTOR, LOW);
  motorActive = false;
}

bool motor_isActive() {
  return motorActive;
}

unsigned long motor_pulseDurationMs() {
  return static_cast<unsigned long>(T_MOTOR);
}
