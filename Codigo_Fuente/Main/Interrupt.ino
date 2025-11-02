void IRAM_ATTR Contar_Moneda() {
  bool currentState = digitalRead(BUTTON_PIN);

  // Detectar solo flanco de bajada REAL
  if (currentState == LOW && lastState == HIGH) {
    unsigned long now = micros();

    if (now - lastValidTime > minPulseInterval) {
      Counter++;        
      MemwriteLimit++;
      lastValidTime = now;
    }
  }

  lastState = currentState;
}