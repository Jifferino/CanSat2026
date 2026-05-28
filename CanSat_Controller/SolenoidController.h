#pragma once
#include <Arduino.h>
#include "config.h"

class SolenoidController {
public:
  void begin();
  void update();

  bool isArmed() const;
  void arm();
  void disarm();

  bool pulse(uint8_t channelOneIndexed, unsigned long durationMs);
  bool isActive(uint8_t channelOneIndexed) const;
  unsigned long remainingMs(uint8_t channelOneIndexed) const;

private:
  bool armed = false;
  bool active[SOLENOID_COUNT] = {false};
  unsigned long offAtMs[SOLENOID_COUNT] = {0};
  unsigned long lastPulseMs[SOLENOID_COUNT] = {0};

  bool validChannel(uint8_t channelOneIndexed) const;
  void setChannel(uint8_t indexZeroBased, bool on);
};
