#include <Arduino.h>
#include <WiFiS3.h>
#include "ProtocolMessage.h"
#include "BafferRead.h"

bool readAndParseMessage(uint8_t buffer[], WiFiClient& client, Protocol::ProtocolMessage& msg) {
  const size_t MSG_SIZE = 8;
  size_t bytesRead = 0;
  unsigned long start = millis();

  while (bytesRead < MSG_SIZE) {
    if (client.available()) {
      buffer[bytesRead++] = client.read();
    }
    if (millis() - start > 1000) {
      Serial.println("Timeout while reading message");
      client.stop();
      return false;
    }
  }

  Protocol::Parser::ParseResult result = Protocol::Parser::tryParseMessageFromBuffer(buffer, MSG_SIZE, msg);
  if (result == Protocol::Parser::ParseResult::OK) return true;

  if (result == Protocol::Parser::ParseResult::UnknownFlag) {
    Serial.println("Unknown flag, trying resync");

    int flagIndex = findValidFlagIndex(buffer, MSG_SIZE);
    if (flagIndex == -1) {
      Serial.println("No valid flag found, discarding buffer");
      return false;
    }

    size_t remain = MSG_SIZE - flagIndex;
    for (size_t i = 0; i < remain; ++i) {
      buffer[i] = buffer[flagIndex + i];
    }

    bytesRead = remain;
    start = millis();

    while (bytesRead < MSG_SIZE) {
      if (client.available()) {
        buffer[bytesRead++] = client.read();
      }
      if (millis() - start > 1000) {
        Serial.println("Timeout during resync");
        client.stop();
        return false;
      }
    }

    result = Protocol::Parser::tryParseMessageFromBuffer(buffer, MSG_SIZE, msg);
    if (result == Protocol::Parser::ParseResult::OK) return true;
    Serial.print("Parse failed after resync: ");
    Serial.println(static_cast<int>(result));
    return false;
  }

  Serial.print("Parse failed: ");
  Serial.println(static_cast<int>(result));
  return false;
}


int findValidFlagIndex(const uint8_t* buffer, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    uint8_t b = buffer[i];
    if (b == Protocol::Constants::F_BTN || b == Protocol::Constants::F_REQ ||
        b == Protocol::Constants::F_RES || b == Protocol::Constants::F_SYNC_R ||
        b == Protocol::Constants::F_SYNC_S) {
      return i;
    }
  }
  return -1;
}
