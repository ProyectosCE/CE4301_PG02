#include <Servo.h>

Servo servoMotor;

// Ángulos de operación del servo
const int SERVO_CERRADO = 0;      // tapa cerrada
const int SERVO_ABIERTO = 170;    // tapa abierta

// Velocidad del barrido (delay entre pasos)
const int SERVO_STEP_DELAY = 8;   // milisegundos

// Inicialización del servo
/* Function: servo_init
   Inicializa el servo y lo coloca en la posición cerrada por defecto.

   Params:
     - Ninguno.

   Returns:
     - void - no retorna valor.
*/
void servo_init() {
  servoMotor.attach(PIN_SERVO);
  servoMotor.write(SERVO_CERRADO);
  delay(300);
}

// Movimiento suave hacia la posición abierta
/* Function: servo_irAAbierto
   Realiza un barrido suave hasta la posición abierta de la compuerta.

   Params:
     - Ninguno.

   Returns:
     - void - no retorna valor.

   Restriction:
     Debe evitarse el llamado concurrente mientras el servo está en movimiento.
*/
void servo_irAAbierto() {
  for (int ang = SERVO_CERRADO; ang <= SERVO_ABIERTO; ang++) {
    servoMotor.write(ang);
    delay(SERVO_STEP_DELAY);
  }
}

// Movimiento suave hacia la posición cerrada
/* Function: servo_irACerrado
   Retorna gradualmente el servo a la posición de cierre.

   Params:
     - Ninguno.

   Returns:
     - void - no retorna valor.

   Restriction:
     Debe ejecutarse luego de finalizar los movimientos de apertura.
*/
void servo_irACerrado() {
  for (int ang = SERVO_ABIERTO; ang >= SERVO_CERRADO; ang--) {
    servoMotor.write(ang);
    delay(SERVO_STEP_DELAY);
  }
}
