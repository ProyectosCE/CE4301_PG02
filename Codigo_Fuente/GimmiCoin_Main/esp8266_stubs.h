#pragma once

#if __has_include(<Arduino.h>)
#include <Arduino.h>
#else
#include "arduino_stub.h"
#endif

class ESP8266WebServer {
 public:
  explicit ESP8266WebServer(int) {}
  void on(const char*, void (*)(void)) {}
  void begin() {}
  void handleClient() {}
  bool authenticate(const char*, const char*) { return true; }
  void requestAuthentication() {}
  void send(int, const char*, const String&) {}
};

class WiFiClass {
 public:
  void softAP(const char*, const char*) {}
  IPAddress softAPIP() { return IPAddress(); }
};

inline WiFiClass WiFi;

