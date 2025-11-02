void IRAM_ATTR Contar_Pulsos() {
  unsigned long currentTime = micros();

  // Debounce de 120 ms (120000 microsegundos)
  if (currentTime - lastInterruptTime > 130000) {
    Counter++;
    MemwriteLimit++;
  }

  lastInterruptTime = currentTime;
}