# CanSat Arduino Starter: ESP32 Feather V2 + ADXL375 + Solenoid Outputs

This is a preliminary Arduino project for a CanSat payload/controller using:

- Adafruit ESP32 Feather V2
- Adafruit ADXL375 high-G accelerometer breakout over I2C
- Up to 4 solenoid driver channels through external MOSFET/driver hardware

## Safety intent

This starter code is for bench testing and CanSat payload/container release mechanisms using electrically actuated solenoids. It is **not** written for pyrotechnics, explosive charges, or autonomous rocket-stage separation. Outputs boot disarmed, require a typed arming command, have pulse-time limits, and automatically turn off after each pulse.

## Folder layout

Open the `CanSat_Controller` folder in the Arduino IDE. The `.ino` file must stay inside a folder with the same name.

```text
CanSat_Controller/
  CanSat_Controller.ino
  config.h
  ADXL375Reader.h
  ADXL375Reader.cpp
  SolenoidController.h
  SolenoidController.cpp
```

## Arduino libraries to install

In Arduino IDE Library Manager, install:

- `Adafruit ADXL375`
- `Adafruit Unified Sensor`

Also install/select the ESP32 board support package and choose the Adafruit ESP32 Feather V2 or matching ESP32 board profile.

## Basic wiring

### ADXL375 over I2C

For the ESP32 Feather V2:

- Feather `3V` -> ADXL375 `VIN`
- Feather `GND` -> ADXL375 `GND`
- Feather `SCL` -> ADXL375 `SCL`
- Feather `SDA` -> ADXL375 `SDA`

Default ADXL375 I2C address: `0x53`.

### Solenoids

Do **not** connect a solenoid directly to an ESP32 GPIO pin. Each solenoid channel should use an external driver, usually:

- logic-level N-channel MOSFET or dedicated solenoid driver
- flyback diode across the solenoid coil
- separate solenoid power supply sized for the coil current
- common ground between Feather and driver power supply

Default GPIO control pins are in `config.h`:

```cpp
static const uint8_t SOLENOID_PINS[SOLENOID_COUNT] = {27, 33, 32, 13};
```

Change these after checking your final wiring.

## Serial commands

Open Serial Monitor at `115200 baud`, newline enabled.

```text
HELP                 show commands
STATUS               print arming state and solenoid states
ARM CANSAT           arm solenoid outputs
DISARM               disarm and force all outputs off
FIRE <ch> [ms]       pulse one solenoid channel, e.g. FIRE 1 250
FIREALL [ms]         pulse all channels for bench testing
LOG ON               enable telemetry printing
LOG OFF              disable telemetry printing
```

Telemetry prints as:

```text
ms,armed,ax_g,ay_g,az_g,total_g,sol1,sol2,sol3,sol4
```

## Before real integration

- Replace default GPIO assignments with pins verified on your final wiring diagram.
- Bench test each solenoid channel with an LED first, then with the driver and solenoid.
- Measure solenoid current and confirm the MOSFET/driver, diode, battery, and wiring are sized correctly.
- Add hardware inhibits, remove-before-flight pins, and mission-rule-compliant arming procedures.
- Consider logging to SD and integrating your competition telemetry format later.
