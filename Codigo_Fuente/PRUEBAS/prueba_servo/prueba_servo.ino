#include <Arduino.h>
#include <Servo.h>

Servo servo;

const int SERVO_PIN = D1;  // Ahora usamos D1 (GPIO5)

void setup() {
  Serial.begin(115200);
  Serial.println("Prueba de servo SG92R en D1 mini");

  // Asociar el servo al pin D1 con rango de pulsos típico
  // 500–2400 microsegundos suele ir bien para Tower Pro
  servo.attach(SERVO_PIN, 500, 2400);

  // Posición inicial
  servo.write(0);
  delay(1000);
}

void loop() {
  // Barrido 0 -> 180
  for (int ang = 0; ang <= 180; ang += 10) {
    servo.write(ang);
    delay(300);
  }

  delay(700);

  // Barrido 180 -> 0
  for (int ang = 180; ang >= 0; ang -= 10) {
    servo.write(ang);
    delay(300);
  }

  delay(1000);
}
