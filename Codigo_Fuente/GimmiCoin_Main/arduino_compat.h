#pragma once

// Determine once whether the translation unit should rely on the real Arduino
// core or fall back to lightweight stubs for tooling support.
#ifndef GM_USING_ARDUINO_CORE
#if defined(ARDUINO) && defined(__XTENSA__)
#define GM_USING_ARDUINO_CORE 1
#else
#define GM_USING_ARDUINO_CORE 0
#endif
#endif

#include "Arduino.h"
