#include "config.h"
#include "ADXL375Reader.h"
#include "SolenoidController.h"

ADXL375Reader accel;
SolenoidController solenoids;

bool telemetryEnabled = true;
unsigned long lastTelemetryMs = 0;
String inputLine;

void printHelp() {
  Serial.println();
  Serial.println("CanSat ESP32 Feather V2 Starter Commands");
  Serial.println("HELP                 - show commands");
  Serial.println("STATUS               - print arming state and solenoid states");
  Serial.println("ARM CANSAT           - arm solenoid outputs");
  Serial.println("DISARM               - disarm and force all outputs off");
  Serial.println("FIRE <ch> [ms]       - pulse one solenoid channel, e.g. FIRE 1 250");
  Serial.println("FIREALL [ms]         - pulse all channels for bench testing");
  Serial.println("LOG ON               - enable telemetry printing");
  Serial.println("LOG OFF              - disable telemetry printing");
  Serial.println();
  Serial.println("Telemetry CSV fields:");
  Serial.println("ms,armed,ax_g,ay_g,az_g,total_g,sol1,sol2,sol3,sol4");
  Serial.println();
}

void printStatus() {
  Serial.print("armed=");
  Serial.print(solenoids.isArmed() ? "YES" : "NO");
  for (uint8_t i = 1; i <= SOLENOID_COUNT; i++) {
    Serial.print(", sol");
    Serial.print(i);
    Serial.print("=");
    Serial.print(solenoids.isActive(i) ? "ON" : "OFF");
    if (solenoids.isActive(i)) {
      Serial.print("(");
      Serial.print(solenoids.remainingMs(i));
      Serial.print("ms left)");
    }
  }
  Serial.println();
}

void printTelemetry() {
  AccelReading r = accel.read();

  Serial.print(millis());
  Serial.print(',');
  Serial.print(solenoids.isArmed() ? 1 : 0);
  Serial.print(',');
  Serial.print(r.ax_g, 3);
  Serial.print(',');
  Serial.print(r.ay_g, 3);
  Serial.print(',');
  Serial.print(r.az_g, 3);
  Serial.print(',');
  Serial.print(r.total_g, 3);

  for (uint8_t i = 1; i <= SOLENOID_COUNT; i++) {
    Serial.print(',');
    Serial.print(solenoids.isActive(i) ? 1 : 0);
  }
  Serial.println();
}

void handleCommand(String cmd) {
  cmd.trim();
  cmd.toUpperCase();
  if (cmd.length() == 0) return;

  if (cmd == "HELP") {
    printHelp();
  } else if (cmd == "STATUS") {
    printStatus();
  } else if (cmd == "ARM CANSAT") {
    solenoids.arm();
    Serial.println("OK: armed");
  } else if (cmd == "DISARM") {
    solenoids.disarm();
    Serial.println("OK: disarmed; all outputs off");
  } else if (cmd == "LOG ON") {
    telemetryEnabled = true;
    Serial.println("OK: telemetry on");
  } else if (cmd == "LOG OFF") {
    telemetryEnabled = false;
    Serial.println("OK: telemetry off");
  } else if (cmd.startsWith("FIREALL")) {
    unsigned long duration = DEFAULT_PULSE_MS;
    int firstSpace = cmd.indexOf(' ');
    if (firstSpace > 0) duration = cmd.substring(firstSpace + 1).toInt();

    bool allOk = true;
    for (uint8_t ch = 1; ch <= SOLENOID_COUNT; ch++) {
      allOk = solenoids.pulse(ch, duration) && allOk;
    }
    Serial.println(allOk ? "OK: fired all requested channels" : "ERR: not armed, invalid duration, or rate limited");
  } else if (cmd.startsWith("FIRE ")) {
    int firstSpace = cmd.indexOf(' ');
    int secondSpace = cmd.indexOf(' ', firstSpace + 1);

    uint8_t ch = 0;
    unsigned long duration = DEFAULT_PULSE_MS;

    if (secondSpace > 0) {
      ch = cmd.substring(firstSpace + 1, secondSpace).toInt();
      duration = cmd.substring(secondSpace + 1).toInt();
    } else {
      ch = cmd.substring(firstSpace + 1).toInt();
    }

    bool ok = solenoids.pulse(ch, duration);
    Serial.println(ok ? "OK: pulse started" : "ERR: not armed, bad channel/duration, or rate limited");
  } else {
    Serial.println("ERR: unknown command. Type HELP.");
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(500);

  solenoids.begin();

  Serial.println("CanSat ESP32 Feather V2 Starter Booting...");
  Serial.println("Outputs start DISARMED by default.");

  if (!accel.begin()) {
    Serial.println("ERR: ADXL375 not detected. Check VIN, GND, SDA, SCL, and I2C address.");
  } else {
    Serial.println("OK: ADXL375 detected.");
  }

  printHelp();
}

void loop() {
  solenoids.update();

  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (inputLine.length() > 0) {
        handleCommand(inputLine);
        inputLine = "";
      }
    } else {
      inputLine += c;
    }
  }

  const unsigned long now = millis();
  if (telemetryEnabled && now - lastTelemetryMs >= TELEMETRY_PERIOD_MS) {
    lastTelemetryMs = now;
    printTelemetry();
  }
}
