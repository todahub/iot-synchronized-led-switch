#ifndef BAFFERREAD_H
#define BAFFERREAD_H

#include <Arduino.h>
#include <WiFiS3.h>
#include "ProtocolMessage.h"

struct UDPState {
  IPAddress remoteIP;
  uint16_t remotePort;
};

// UDPStateの外部変数宣言（実体は.cpp側で定義）
extern UDPState udpState;

bool readAndParseMessage(uint8_t buffer[], WiFiClient& client, Protocol::ProtocolMessage& msg);
bool readAndParseMessageUDP(uint8_t buffer[],WiFiUDP& udp, Protocol::ProtocolMessage& msg);
int findValidFlagIndex(const uint8_t* buffer, size_t len);

#endif // BAFFERREAD_H
