# Wiring Notes / Checklist

## ADXL375

- Use I2C first because it keeps wiring simple.
- Default I2C address: `0x53`.
- On ESP32 Feather V2 with Arduino, use the board's `SDA` and `SCL` pins with `Wire`.
- Power from Feather `3V`, not from an unregulated battery rail.

## Solenoid driver checklist

Each solenoid channel should have:

- ESP32 GPIO -> resistor/gate input -> MOSFET or driver input
- Solenoid connected to external solenoid supply, not ESP32 3V pin
- Flyback diode across coil, oriented so it does not conduct during normal energizing
- Common ground between Feather and solenoid driver supply
- Physical safe/arm switch or removable jumper before flight/integration

## Initial bench test sequence

1. Upload the starter code with no solenoids connected.
2. Confirm ADXL375 appears in Serial Monitor.
3. Connect LEDs/resistors to the solenoid output pins and test `FIRE 1 250`, etc.
4. Replace LEDs with MOSFET driver inputs.
5. Power solenoids from a current-limited bench supply first.
6. Confirm the driver remains cool and voltage does not brown out the ESP32.
