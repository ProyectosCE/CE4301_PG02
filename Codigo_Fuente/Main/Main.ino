#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>


//-------------------------------------------- WEB ----------------------------------------------------------------
const char* ssid = "Pajaros_universales"; //nombre de la red
const char* password = "12345678"; //contrasenia 

const char* user = "admin"; //para posible seguridad web
const char* pass = "1234"; //posible seguridad web
ESP8266WebServer server(80);

//--------------------------------------------PINES-------------------------------------------------------------------
const int BUTTON_PIN = 5; // GPIO5 = D1



//-------------------------------------------VAR_INTER---------------------------------------------------------------

volatile unsigned int Counter = 0; //Contador de presiones, monedas, etc
volatile unsigned int MemwriteLimit = 0; //cada cuantas presiones va a escribir en memoria
volatile bool buttonPressed = false; //estado del boton


void IRAM_ATTR Contar_Moneda();

volatile unsigned long lastInterruptTime = 0; //contador para debouncer

const int memDir = 0; //direccion donde se guarda el Counter

volatile bool lastState = HIGH; // con pull-up interno
volatile unsigned long lastValidTime = 0;
const unsigned long minPulseInterval = 9000000; // para evitar doble conteo



void setup() {
  Serial.begin(115200);

  Start_server( ssid,  password);
  Serial.println("Access Point creado ");
  Serial.println(WiFi.softAPIP());

  pinMode(BUTTON_PIN, INPUT_PULLUP); // pin del boton


  EEPROM.begin(10); //reservamos 10 bytes, 
  Counter = EEPROM.read(memDir);
  Serial.print("Valor recuperado: ");
  Serial.println(Counter);



  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), Contar_Moneda, CHANGE); //interrupcion



}


void Write_Value_Mem(int Direc, int Value){
      EEPROM.write(Direc, Value);
      EEPROM.commit();
}



void loop() {
  server.handleClient(); //cada 10 pulsaciones escribe en memoria, para salvar ciclos
   if (MemwriteLimit >= 10) {
    Write_Value_Mem(memDir,Counter);
    MemwriteLimit = 0; //se resetea el counter 
    Serial.println("Guardado en EEPROM ✅");
  }



}
