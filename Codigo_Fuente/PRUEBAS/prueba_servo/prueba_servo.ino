#include <Arduino.h>
#include <Servo.h>

Servo servo;

const int SERVO_PIN  = D1;    
const int MAX_ANGLE  = 90;   
const int STEP       = 5;     // Tamaño del paso (más pequeño = más suave)
const int STEP_DELAY = 20;    // ms entre pasos (20ms = velocidad media-alta)

void setup() {
  Serial.begin(115200);
  Serial.println("Barrido suave 0 -> MAX_ANGLE -> 0");

  
  servo.attach(SERVO_PIN, 500, 2400);

  // Posición inicial
  servo.write(0);
  delay(1000);
}

void loop() {
  // Subir: 0 -> MAX_ANGLE
  for (int ang = 0; ang <= MAX_ANGLE; ang += STEP) {
    servo.write(ang);
    delay(STEP_DELAY);
  }

  // Pausa breve en la posición "abierta"
  delay(300);

  // Bajar: MAX_ANGLE -> 0
  for (int ang = MAX_ANGLE; ang >= 0; ang -= STEP) {
    servo.write(ang);
    delay(STEP_DELAY);
  }

  // Pausa en "cerrado"
  delay(500);
}
