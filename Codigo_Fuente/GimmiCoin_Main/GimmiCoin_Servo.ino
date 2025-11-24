#include <Servo.h>

Servo servoMotor;

// Ángulos de operación del servo
const int SERVO_CERRADO = 0;      // tapa cerrada
const int SERVO_ABIERTO = 170;    // tapa abierta

// Velocidad del barrido (delay entre pasos)
const int SERVO_STEP_DELAY = 8;   // milisegundos

// Inicialización del servo
void servo_init() {
  servoMotor.attach(PIN_SERVO);
  servoMotor.write(SERVO_CERRADO);
  delay(300);
}

// Movimiento suave hacia la posición abierta
void servo_irAAbierto() {
  for (int ang = SERVO_CERRADO; ang <= SERVO_ABIERTO; ang++) {
    servoMotor.write(ang);
    delay(SERVO_STEP_DELAY);
  }
}

// Movimiento suave hacia la posición cerrada
void servo_irACerrado() {
  for (int ang = SERVO_ABIERTO; ang >= SERVO_CERRADO; ang--) {
    servoMotor.write(ang);
    delay(SERVO_STEP_DELAY);
  }
}
