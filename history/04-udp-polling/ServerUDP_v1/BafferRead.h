#ifndef BAFFERREAD_H
#define BAFFERREAD_H

#include <Arduino.h>
#include <WiFiS3.h>
#include "ProtocolMessage.h"

bool readAndParseMessage(uint8_t buffer[], WiFiClient& client, Protocol::ProtocolMessage& msg);
int findValidFlagIndex(const uint8_t* buffer, size_t len);

#endif // BAFFERREAD_H
