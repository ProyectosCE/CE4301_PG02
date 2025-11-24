#pragma once

#include <cstdint>

class SoftwareSerial {
 public:
  SoftwareSerial(int, int) {}
  void begin(unsigned long) {}
};

class DFRobotDFPlayerMini {
 public:
  bool begin(SoftwareSerial&) { return true; }
  void setTimeOut(uint16_t) {}
  void volume(uint8_t) {}
  void EQ(uint8_t) {}
  void outputDevice(uint8_t) {}
  void play(uint16_t) {}
  bool available() { return false; }
  uint8_t readType() { return 0; }
  int read() { return 0; }
};

enum {
  DFPLAYER_EQ_NORMAL = 0,
  DFPLAYER_DEVICE_SD = 2,
  TimeOut = 1,
  WrongStack = 2,
  DFPlayerCardInserted = 3,
  DFPlayerCardRemoved = 4,
  DFPlayerCardOnline = 5,
  DFPlayerPlayFinished = 6,
  DFPlayerError = 7,
  Busy = 8,
  Sleeping = 9,
  SerialWrongStack = 10,
  CheckSumNotMatch = 11,
  FileIndexOut = 12,
  FileMismatch = 13,
  Advertise = 14
};
