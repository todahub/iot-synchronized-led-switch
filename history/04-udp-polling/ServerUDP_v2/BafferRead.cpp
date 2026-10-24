#include <Arduino.h>
#include <WiFiS3.h>
#include "ProtocolMessage.h"
#include "BafferRead.h"


UDPState udpState;  // ここで実体を定義


bool readAndParseMessage(uint8_t buffer[], WiFiClient& client, Protocol::ProtocolMessage& msg) {
  const size_t MSG_SIZE = 8;
  size_t bytesRead = 0;
  unsigned long start = millis();

  while (bytesRead < MSG_SIZE) {
    if (client.available()) {
      // まとめて読めるバイト数を取得して、残りサイズと比較
      size_t available = client.available();
      size_t toRead = min(MSG_SIZE - bytesRead, available);
      bytesRead += client.read(buffer + bytesRead, toRead);
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

bool readAndParseMessageUDP(uint8_t buffer[],WiFiUDP& udp, Protocol::ProtocolMessage& msg) {
  const size_t MSG_SIZE = 8;

  int packetSize = udp.parsePacket();
  if (packetSize < MSG_SIZE) {
    if (packetSize > 0) {
      udp.read(buffer, packetSize);  // 読み捨て
      Serial.println("UDP packet too short");
    }
    return false;
  }

  udpState.remoteIP = udp.remoteIP();
  udpState.remotePort = udp.remotePort();

  udp.read(buffer, MSG_SIZE);  // 8バイト分読み込み

  Protocol::Parser::ParseResult result = Protocol::Parser::tryParseMessageFromBuffer(buffer, MSG_SIZE, msg);
  if (result == Protocol::Parser::ParseResult::OK) return true;

  if (result == Protocol::Parser::ParseResult::UnknownFlag) {
    Serial.println("Unknown flag, trying resync");

    int flagIndex = findValidFlagIndex(buffer, MSG_SIZE);
    if (flagIndex == -1) {
      Serial.println("No valid flag found, discarding packet");
      return false;
    }

    // 再同期処理
    size_t remain = MSG_SIZE - flagIndex;
    for (size_t i = 0; i < remain; ++i) {
      buffer[i] = buffer[flagIndex + i];//正しいflagを先頭に
    }

    int moreSize = udp.read(buffer + remain, MSG_SIZE - remain);//buffer + remainでbufferのremain番目の要素から入れれる
    if (moreSize != (MSG_SIZE - remain)) {
      Serial.println("Not enough data to complete message after resync");
      return false;
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
