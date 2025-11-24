#pragma once

#if defined(ARDUINO) && defined(__XTENSA__)
#include_next <ESP8266WiFi.h>
#else
#include "esp8266_stubs.h"
#endif
