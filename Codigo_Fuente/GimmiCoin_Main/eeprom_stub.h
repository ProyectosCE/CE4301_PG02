#pragma once

#if __has_include(<Arduino.h>)
#include <Arduino.h>
#else
#include <cstddef>
#endif

class EEPROMClassStub {
 public:
  bool begin(size_t) { return true; }
  void commit() {}

  template <typename T>
  void put(int, const T&) {}

  template <typename T>
  void get(int, T& value) {
    value = T();
  }
};

inline EEPROMClassStub EEPROM;

