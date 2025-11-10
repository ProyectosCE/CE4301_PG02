
//interrupcion para el boton

void IRAM_ATTR Contar_Pulsos() { //interrupcion cada vez que d1 baja
  unsigned long currentTime = micros(); 

  if (currentTime - lastInterruptTime > 120000) {  //debouncer
    Counter++;
    MemwriteLimit++;
  }

  lastInterruptTime = currentTime;
}