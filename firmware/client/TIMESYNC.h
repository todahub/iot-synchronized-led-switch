#ifndef TIMESYNC_H
#define TIMESYNC_H

#include <Arduino.h>
#include <WiFiS3.h>
#include "ProtocolMessage.h"  // Protocol関係の宣言があればインクルード

extern unsigned long clientTime;

void doTimeSync(WiFiClient& client);

void doTimeSync();

#endif
