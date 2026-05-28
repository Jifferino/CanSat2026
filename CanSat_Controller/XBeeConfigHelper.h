#pragma once
#include <Arduino.h>
#include "config.h"

// Optional helper for bench configuration checks in XBee transparent/AT mode.
// It temporarily enters local AT command mode using the default +++ guard-time method.
// Do not call this during time-critical operation because it pauses radio traffic.
class XBeeConfigHelper {
public:
  bool enterCommandMode(Stream &debugPort, unsigned long guardMs = 1100);
  bool sendAT(const String &command, String &response, unsigned long timeoutMs = 1000);
  void exitCommandMode();
};
