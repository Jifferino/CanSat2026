/*
  GroundStation_XBee_Bridge

  Upload this to a second ESP32 Feather V2 connected to another XB24CDMSIT-001
  XBee module. It bridges your computer's Serial Monitor to the XBee UART.

  Type radio commands in Serial Monitor, for example:
    CMD,0000,PING
    CMD,0000,CX,ON
    CMD,0000,STATUS
    CMD,0000,DISARM

  Set TEAM_ID in the CanSat sketch and use the same value here in your commands.
*/

static const unsigned long USB_BAUD = 115200;
static const unsigned long XBEE_BAUD = 9600;
static const uint8_t XBEE_RX_PIN = 16;  // ESP32 receives from XBee DOUT
static const uint8_t XBEE_TX_PIN = 17;  // ESP32 transmits to XBee DIN
#define XBEE_SERIAL Serial1

String usbLine;

uint8_t xorChecksum(const String &s) {
  uint8_t cs = 0;
  for (size_t i = 0; i < s.length(); i++) cs ^= (uint8_t)s[i];
  return cs;
}

String checksumHex(uint8_t value) {
  const char hex[] = "0123456789ABCDEF";
  String out;
  out += hex[(value >> 4) & 0x0F];
  out += hex[value & 0x0F];
  return out;
}

void sendCommand(String line) {
  line.trim();
  if (line.length() == 0) return;

  // If the user already typed a framed packet, send it as-is.
  // Otherwise, add $...*XX framing for simple checksum protection.
  if (line.startsWith("$")) {
    XBEE_SERIAL.print(line);
    XBEE_SERIAL.print('\r');
  } else {
    String framed = "$" + line + "*" + checksumHex(xorChecksum(line));
    XBEE_SERIAL.print(framed);
    XBEE_SERIAL.print('\r');
    Serial.print("sent: ");
    Serial.println(framed);
  }
}

void setup() {
  Serial.begin(USB_BAUD);
  delay(500);
  XBEE_SERIAL.begin(XBEE_BAUD, SERIAL_8N1, XBEE_RX_PIN, XBEE_TX_PIN);
  Serial.println("Ground Station XBee Bridge ready.");
  Serial.println("Type commands like: CMD,0000,PING or CMD,0000,CX,ON");
}

void loop() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      sendCommand(usbLine);
      usbLine = "";
    } else {
      usbLine += c;
    }
  }

  while (XBEE_SERIAL.available() > 0) {
    char c = (char)XBEE_SERIAL.read();
    Serial.write(c);
    if (c == '\r') Serial.write('\n');
  }
}
