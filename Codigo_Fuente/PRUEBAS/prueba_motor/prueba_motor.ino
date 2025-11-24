int motorPin = D8;

void setup() {
  pinMode(motorPin, OUTPUT);
}

void loop() {
  digitalWrite(motorPin, HIGH);  // Enciende motor
  delay(2000);                   // 2 segundos

  digitalWrite(motorPin, LOW);   // Apaga motor
  delay(2000);                   // 2 segundos
}
