#pragma once

// =============================
// CanSat Arduino Starter Config
// Board: Adafruit ESP32 Feather V2
// Sensor: Adafruit ADXL375 High-G Accelerometer
// =============================

// Serial settings
static const unsigned long SERIAL_BAUD = 115200;
static const unsigned long TELEMETRY_PERIOD_MS = 100;  // 10 Hz for bench testing

// ADXL375 settings
static const uint8_t ADXL375_I2C_ADDR = 0x53;          // Default Adafruit ADXL375 address
static const float STANDARD_GRAVITY = 9.80665f;        // m/s^2 per g

// Solenoid driver settings
// IMPORTANT: These pins are logic-level control pins for MOSFET/driver inputs only.
// Do NOT drive solenoids directly from ESP32 GPIO pins.
// Avoid I2C pins: SDA=22, SCL=20 on ESP32 Feather V2.
static const uint8_t SOLENOID_COUNT = 4;
static const uint8_t SOLENOID_PINS[SOLENOID_COUNT] = {27, 33, 32, 13};

// Use HIGH if your MOSFET/driver input turns ON when GPIO is HIGH.
// Change to LOW only if your driver board is active-low.
static const uint8_t SOLENOID_ACTIVE_STATE = HIGH;
static const uint8_t SOLENOID_INACTIVE_STATE = LOW;

// Safety limits
static const unsigned long MAX_SOLENOID_PULSE_MS = 1000;   // hard cap per pulse
static const unsigned long DEFAULT_PULSE_MS = 250;          // starter test pulse
static const unsigned long MIN_TIME_BETWEEN_PULSES_MS = 2000;

// Arming phrase used over Serial Monitor.
// Type exactly: ARM CANSAT
static const char ARM_PHRASE[] = "CANSAT";
