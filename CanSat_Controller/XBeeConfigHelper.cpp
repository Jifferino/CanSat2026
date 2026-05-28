#include "XBeeConfigHelper.h"

bool XBeeConfigHelper::enterCommandMode(Stream &debugPort, unsigned long guardMs) {
#if RADIO_ENABLED
  delay(guardMs);
  XBEE_SERIAL.print("+++");
  delay(guardMs);

  String response;
  unsigned long start = millis();
  while (millis() - start < 1500) {
    while (XBEE_SERIAL.available() > 0) {
      char c = (char)XBEE_SERIAL.read();
      response += c;
      if (response.indexOf("OK") >= 0) {
        debugPort.println("OK: XBee entered command mode");
        return true;
      }
    }
  }
  debugPort.print("ERR: XBee command mode failed, response=");
  debugPort.println(response);
#endif
  return false;
}

bool XBeeConfigHelper::sendAT(const String &command, String &response, unsigned long timeoutMs) {
#if RADIO_ENABLED
  response = "";
  XBEE_SERIAL.print("AT");
  XBEE_SERIAL.print(command);
  XBEE_SERIAL.print('\r');

  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    while (XBEE_SERIAL.available() > 0) {
      char c = (char)XBEE_SERIAL.read();
      response += c;
      if (c == '\r' || c == '\n') return response.length() > 0;
    }
  }
#endif
  return false;
}

void XBeeConfigHelper::exitCommandMode() {
#if RADIO_ENABLED
  XBEE_SERIAL.print("ATCN\r");
#endif
}
