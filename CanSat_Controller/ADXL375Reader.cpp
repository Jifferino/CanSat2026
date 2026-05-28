#include "ADXL375Reader.h"
#include <math.h>

bool ADXL375Reader::begin() {
  Wire.begin();
  return accel.begin(ADXL375_I2C_ADDR);
}

AccelReading ADXL375Reader::read() {
  sensors_event_t event;
  AccelReading r;
  r.valid = accel.getEvent(&event);

  if (!r.valid) {
    r.ax_ms2 = r.ay_ms2 = r.az_ms2 = 0.0f;
    r.ax_g = r.ay_g = r.az_g = r.total_g = 0.0f;
    return r;
  }

  r.ax_ms2 = event.acceleration.x;
  r.ay_ms2 = event.acceleration.y;
  r.az_ms2 = event.acceleration.z;

  r.ax_g = r.ax_ms2 / STANDARD_GRAVITY;
  r.ay_g = r.ay_ms2 / STANDARD_GRAVITY;
  r.az_g = r.az_ms2 / STANDARD_GRAVITY;
  r.total_g = sqrtf(r.ax_g * r.ax_g + r.ay_g * r.ay_g + r.az_g * r.az_g);

  return r;
}
