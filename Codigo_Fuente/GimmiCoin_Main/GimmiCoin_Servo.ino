// =====================================================================================
// Módulo del servo - Control de compuerta de monedas
// =====================================================================================

#if defined(ARDUINO)
#include <Servo.h>
static Servo servoMotor;
#else
class Servo {
public:
  void attach(int) {}
  void write(int) {}
  void detach() {}
};
static Servo servoMotor;
#endif

void servo_init() {
  servoMotor.attach(PIN_SERVO);
  servo_irACerrado();
  Serial.println(F("[SERVO] Servo inicializado en posición cerrada."));
}

void servo_irAAbierto() {
  servoMotor.write(90);  // Posición abierta
  Serial.println(F("[SERVO] Posición: ABIERTO"));
}

void servo_irACerrado() {
  servoMotor.write(0);   // Posición cerrada
  Serial.println(F("[SERVO] Posición: CERRADO"));
}