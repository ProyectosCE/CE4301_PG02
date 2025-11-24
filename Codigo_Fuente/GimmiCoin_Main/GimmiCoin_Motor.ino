// Duración del pulso del motor en milisegundos
const int T_MOTOR = 2000;

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
  digitalWrite(PIN_MOTOR, HIGH);
  delay(T_MOTOR);
  digitalWrite(PIN_MOTOR, LOW);
}
