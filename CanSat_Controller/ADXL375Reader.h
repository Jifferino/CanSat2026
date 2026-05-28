#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL375.h>
#include "config.h"

struct AccelReading {
  float ax_ms2;
  float ay_ms2;
  float az_ms2;
  float ax_g;
  float ay_g;
  float az_g;
  float total_g;
  bool valid;
};

class ADXL375Reader {
public:
  bool begin();
  AccelReading read();

private:
  Adafruit_ADXL375 accel = Adafruit_ADXL375(375);
};
