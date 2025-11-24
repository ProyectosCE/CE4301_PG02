#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <utility>

class String : public std::string {
 public:
  using std::string::string;

  String() = default;
  String(const char* s) : std::string(s ? s : "") {}
  String(const std::string& other) : std::string(other) {}
  String(std::string&& other) noexcept : std::string(std::move(other)) {}
  String(unsigned long value) : std::string(std::to_string(value)) {}
  String(long value) : std::string(std::to_string(value)) {}
  String(unsigned int value) : std::string(std::to_string(value)) {}
  String(int value) : std::string(std::to_string(value)) {}
  String(float value) : std::string(std::to_string(value)) {}
  String(double value) : std::string(std::to_string(value)) {}
};

#ifndef F
#define F(x) x
#endif

#ifndef PROGMEM
#define PROGMEM
#endif

#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

// NodeMCU style pin aliases (approximate)
#ifndef D0
#define D0 16
#endif
#ifndef D1
#define D1 5
#endif
#ifndef D2
#define D2 4
#endif
#ifndef D3
#define D3 0
#endif
#ifndef D4
#define D4 2
#endif
#ifndef D5
#define D5 14
#endif
#ifndef D6
#define D6 12
#endif
#ifndef D7
#define D7 13
#endif
#ifndef D8
#define D8 15
#endif

inline unsigned long millis() { return 0; }
inline unsigned long micros() { return 0; }
inline void delay(unsigned long) {}
inline void yield() {}

inline void noInterrupts() {}
inline void interrupts() {}

inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}
inline int digitalRead(int) { return 0; }
inline void attachInterrupt(int, void (*)(void), int) {}
inline int digitalPinToInterrupt(int pin) { return pin; }

class IPAddress {
 public:
  IPAddress() = default;
  std::string toString() const { return "0.0.0.0"; }
};

class HardwareSerialStub {
 public:
  template <typename T>
  void print(const T&) {}

  template <typename T>
  void println(const T&) {}

  void println() {}

  template <typename T>
  void print(const T&, int) {}

  template <typename T>
  void println(const T&, int) {}

  void begin(unsigned long) {}
};

inline HardwareSerialStub Serial;

const int INPUT = 0;
const int OUTPUT = 1;
const int INPUT_PULLUP = 2;

const int CHANGE = 3;
const int RISING = 4;
const int FALLING = 5;

const int LOW = 0;
const int HIGH = 1;

inline int analogRead(int) { return 0; }
