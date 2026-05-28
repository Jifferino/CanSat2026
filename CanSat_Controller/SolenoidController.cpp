#include "SolenoidController.h"

void SolenoidController::begin() {
  for (uint8_t i = 0; i < SOLENOID_COUNT; i++) {
    pinMode(SOLENOID_PINS[i], OUTPUT);
    digitalWrite(SOLENOID_PINS[i], SOLENOID_INACTIVE_STATE);
    active[i] = false;
    offAtMs[i] = 0;
    lastPulseMs[i] = 0;
  }
  armed = false; // always boot disarmed
}

void SolenoidController::update() {
  const unsigned long now = millis();
  for (uint8_t i = 0; i < SOLENOID_COUNT; i++) {
    if (active[i] && (long)(now - offAtMs[i]) >= 0) {
      setChannel(i, false);
    }
  }
}

bool SolenoidController::isArmed() const {
  return armed;
}

void SolenoidController::arm() {
  armed = true;
}

void SolenoidController::disarm() {
  armed = false;
  for (uint8_t i = 0; i < SOLENOID_COUNT; i++) {
    setChannel(i, false);
  }
}

bool SolenoidController::pulse(uint8_t channelOneIndexed, unsigned long durationMs) {
  if (!armed) return false;
  if (!validChannel(channelOneIndexed)) return false;
  if (durationMs == 0 || durationMs > MAX_SOLENOID_PULSE_MS) return false;

  const uint8_t i = channelOneIndexed - 1;
  const unsigned long now = millis();
  if (lastPulseMs[i] != 0 && now - lastPulseMs[i] < MIN_TIME_BETWEEN_PULSES_MS) {
    return false;
  }

  setChannel(i, true);
  offAtMs[i] = now + durationMs;
  lastPulseMs[i] = now;
  return true;
}

bool SolenoidController::isActive(uint8_t channelOneIndexed) const {
  if (!validChannel(channelOneIndexed)) return false;
  return active[channelOneIndexed - 1];
}

unsigned long SolenoidController::remainingMs(uint8_t channelOneIndexed) const {
  if (!validChannel(channelOneIndexed)) return 0;
  const uint8_t i = channelOneIndexed - 1;
  if (!active[i]) return 0;
  const unsigned long now = millis();
  if ((long)(offAtMs[i] - now) <= 0) return 0;
  return offAtMs[i] - now;
}

bool SolenoidController::validChannel(uint8_t channelOneIndexed) const {
  return channelOneIndexed >= 1 && channelOneIndexed <= SOLENOID_COUNT;
}

void SolenoidController::setChannel(uint8_t indexZeroBased, bool on) {
  active[indexZeroBased] = on;
  digitalWrite(SOLENOID_PINS[indexZeroBased], on ? SOLENOID_ACTIVE_STATE : SOLENOID_INACTIVE_STATE);
}
