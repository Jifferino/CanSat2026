#include "config.h"
#include "ADXL375Reader.h"
#include "SolenoidController.h"
#include "XBeeConfigHelper.h"

ADXL375Reader accel;
SolenoidController solenoids;
RadioLink radio;
XBeeConfigHelper xbeeConfig;

bool telemetryEnabled = true;
unsigned long lastTelemetryMs = 0;
String inputLine;

String latestTelemetryCsv() {
  AccelReading r = accel.read();
  String csv;
  csv.reserve(96);
  csv += String(millis());
  csv += ',';
  csv += solenoids.isArmed() ? '1' : '0';
  csv += ',';
  csv += String(r.ax_g, 3);
  csv += ',';
  csv += String(r.ay_g, 3);
  csv += ',';
  csv += String(r.az_g, 3);
  csv += ',';
  csv += String(r.total_g, 3);
  for (uint8_t i = 1; i <= SOLENOID_COUNT; i++) {
    csv += ',';
    csv += solenoids.isActive(i) ? '1' : '0';
  }
  return csv;
}

void printHelp() {
  Serial.println();
  Serial.println("CanSat ESP32 Feather V2 Starter Commands");
  Serial.println("HELP                 - show commands");
  Serial.println("STATUS               - print arming state and solenoid states");
  Serial.println("ARM CANSAT           - arm local solenoid outputs");
  Serial.println("DISARM               - disarm and force all outputs off");
  Serial.println("FIRE <ch> [ms]       - local-only pulse one release channel, e.g. FIRE 1 250");
  Serial.println("FIREALL [ms]         - local-only pulse all channels for bench testing");
  Serial.println("LOG ON               - enable telemetry printing");
  Serial.println("LOG OFF              - disable telemetry printing");
  Serial.println("XBEE CHECK           - enter XBee AT mode and print basic settings");
  Serial.println();
  Serial.println("Radio commands use: CMD,<TEAM_ID>,<COMMAND>[,<ARGS>]");
  Serial.println("Examples: CMD,0000,PING | CMD,0000,CX,ON | CMD,0000,STATUS | CMD,0000,DISARM");
  Serial.println("Remote release pulses are disabled unless RADIO_ALLOW_REMOTE_RELEASE=true in config.h");
  Serial.println();
  Serial.println("USB telemetry CSV fields:");
  Serial.println("ms,armed,ax_g,ay_g,az_g,total_g,sol1,sol2,sol3,sol4");
  Serial.println("Radio telemetry frame: $TLM,TEAM_ID,seq,<same csv>*XX");
  Serial.println();
}

String statusString() {
  String s;
  s.reserve(96);
  s += "armed=";
  s += solenoids.isArmed() ? "YES" : "NO";
  for (uint8_t i = 1; i <= SOLENOID_COUNT; i++) {
    s += ",sol";
    s += String(i);
    s += "=";
    s += solenoids.isActive(i) ? "ON" : "OFF";
    if (solenoids.isActive(i)) {
      s += "(";
      s += String(solenoids.remainingMs(i));
      s += "ms left)";
    }
  }
  return s;
}

void printStatus() {
  Serial.println(statusString());
}

void printTelemetry() {
  Serial.println(latestTelemetryCsv());
}

void handleXBeeCheck() {
  if (!xbeeConfig.enterCommandMode(Serial)) return;

  String response;
  const char *commands[] = {"ID", "CH", "BD", "AP", "MY", "DL", "DH", "NI"};
  for (const char *cmd : commands) {
    if (xbeeConfig.sendAT(cmd, response)) {
      response.trim();
      Serial.print("AT");
      Serial.print(cmd);
      Serial.print(" = ");
      Serial.println(response);
    } else {
      Serial.print("AT");
      Serial.print(cmd);
      Serial.println(" = <no response>");
    }
  }
  xbeeConfig.exitCommandMode();
}

void handleCommand(String cmd, bool fromRadio) {
  cmd.trim();
  cmd.toUpperCase();
  if (cmd.length() == 0) return;

  if (cmd == "HELP") {
    if (fromRadio) radio.sendAck("HELP USB_ONLY"); else printHelp();
  } else if (cmd == "STATUS") {
    if (fromRadio) radio.sendAck(statusString()); else printStatus();
  } else if (cmd == "ARM CANSAT") {
    if (fromRadio && !RADIO_ALLOW_REMOTE_RELEASE) {
      radio.sendErr("REMOTE_ARM_DISABLED");
      return;
    }
    solenoids.arm();
    if (fromRadio) radio.sendAck("ARMED"); else Serial.println("OK: armed");
  } else if (cmd == "DISARM") {
    solenoids.disarm();
    if (fromRadio) radio.sendAck("DISARMED_ALL_OUTPUTS_OFF"); else Serial.println("OK: disarmed; all outputs off");
  } else if (cmd == "LOG ON") {
    telemetryEnabled = true;
    if (fromRadio) radio.sendAck("TELEMETRY_ON"); else Serial.println("OK: telemetry on");
  } else if (cmd == "LOG OFF") {
    telemetryEnabled = false;
    if (fromRadio) radio.sendAck("TELEMETRY_OFF"); else Serial.println("OK: telemetry off");
  } else if (!fromRadio && cmd == "XBEE CHECK") {
    handleXBeeCheck();
  } else if (cmd.startsWith("FIREALL")) {
    if (fromRadio && !RADIO_ALLOW_REMOTE_RELEASE) {
      radio.sendErr("REMOTE_RELEASE_DISABLED");
      return;
    }
    unsigned long duration = DEFAULT_PULSE_MS;
    int firstSpace = cmd.indexOf(' ');
    if (firstSpace > 0) duration = cmd.substring(firstSpace + 1).toInt();

    bool allOk = true;
    for (uint8_t ch = 1; ch <= SOLENOID_COUNT; ch++) {
      allOk = solenoids.pulse(ch, duration) && allOk;
    }
    if (fromRadio) radio.sendAck(allOk ? "FIRED_ALL_REQUESTED_CHANNELS" : "FIREALL_REJECTED");
    else Serial.println(allOk ? "OK: fired all requested channels" : "ERR: not armed, invalid duration, or rate limited");
  } else if (cmd.startsWith("FIRE ")) {
    if (fromRadio && !RADIO_ALLOW_REMOTE_RELEASE) {
      radio.sendErr("REMOTE_RELEASE_DISABLED");
      return;
    }
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
    if (fromRadio) radio.sendAck(ok ? "PULSE_STARTED" : "PULSE_REJECTED");
    else Serial.println(ok ? "OK: pulse started" : "ERR: not armed, bad channel/duration, or rate limited");
  } else {
    if (fromRadio) radio.sendErr("UNKNOWN_COMMAND");
    else Serial.println("ERR: unknown command. Type HELP.");
  }
}

String csvField(const String &s, int fieldIndex) {
  int start = 0;
  int current = 0;
  for (int i = 0; i <= s.length(); i++) {
    if (i == s.length() || s[i] == ',') {
      if (current == fieldIndex) return s.substring(start, i);
      current++;
      start = i + 1;
    }
  }
  return "";
}

void handleRadioPacket(const RadioPacket &packet) {
  if (packet.hasChecksum && !packet.validChecksum) {
    radio.sendErr("BAD_CHECKSUM");
    return;
  }

  String payload = packet.payload;
  payload.trim();
  payload.toUpperCase();

  // Supported inbound radio command format:
  // CMD,<TEAM_ID>,PING
  // CMD,<TEAM_ID>,CX,ON/OFF
  // CMD,<TEAM_ID>,STATUS
  // CMD,<TEAM_ID>,DISARM
  // CMD,<TEAM_ID>,ARM,CANSAT       rejected unless RADIO_ALLOW_REMOTE_RELEASE=true
  // CMD,<TEAM_ID>,FIRE,<ch>,<ms>   rejected unless RADIO_ALLOW_REMOTE_RELEASE=true
  if (csvField(payload, 0) != "CMD") return;
  if (csvField(payload, 1) != String(TEAM_ID)) {
    radio.sendErr("WRONG_TEAM_ID");
    return;
  }

  String op = csvField(payload, 2);
  if (op == "PING") {
    radio.sendAck("PONG");
  } else if (op == "CX") {
    String onOff = csvField(payload, 3);
    if (onOff == "ON") handleCommand("LOG ON", true);
    else if (onOff == "OFF") handleCommand("LOG OFF", true);
    else radio.sendErr("BAD_CX_ARG");
  } else if (op == "STATUS") {
    handleCommand("STATUS", true);
  } else if (op == "DISARM") {
    handleCommand("DISARM", true);
  } else if (op == "ARM") {
    handleCommand("ARM " + csvField(payload, 3), true);
  } else if (op == "FIRE") {
    handleCommand("FIRE " + csvField(payload, 3) + " " + csvField(payload, 4), true);
  } else {
    radio.sendErr("UNKNOWN_CMD_OP");
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(500);

  solenoids.begin();
  radio.begin();

  Serial.println("CanSat ESP32 Feather V2 Starter Booting...");
  Serial.println("Outputs start DISARMED by default.");
  Serial.println("XBee radio link starting on Serial1. Check config.h for RX/TX pins and baud.");

  if (!accel.begin()) {
    Serial.println("ERR: ADXL375 not detected. Check VIN, GND, SDA, SCL, and I2C address.");
  } else {
    Serial.println("OK: ADXL375 detected.");
  }

  radio.sendAck("BOOT");
  printHelp();
}

void loop() {
  solenoids.update();
  radio.update();

  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (inputLine.length() > 0) {
        handleCommand(inputLine, false);
        inputLine = "";
      }
    } else {
      inputLine += c;
    }
  }

  if (radio.available()) {
    handleRadioPacket(radio.readPacket());
  }

  const unsigned long now = millis();
  if (telemetryEnabled && now - lastTelemetryMs >= TELEMETRY_PERIOD_MS) {
    lastTelemetryMs = now;
    String csv = latestTelemetryCsv();
    Serial.println(csv);
    radio.sendTelemetryCsv(csv);
  }
}
