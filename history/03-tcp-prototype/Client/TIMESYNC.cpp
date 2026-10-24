#include "TimeSync.h"
#include <WiFiS3.h> 

unsigned long clientTime = 0;

void doTimeSync(WiFiClient& client) {
  uint8_t txBuffer[8];
  uint8_t rxBuffer[8];
  unsigned long t1 = millis();

  // 同期リクエスト作成
  Protocol::Factory::createTimeSyncRequest(txBuffer, t1);

  // --- 送信 ---
  client.write(txBuffer, 8);

  // --- 応答待ち ---
  delay(100);  // 通信相手が応答するまで少し待機 この時間の調整

  if (client.available() >= 8) {
    client.readBytes(rxBuffer, 8);

    Protocol::ProtocolMessage msg;
    if (Protocol::Parser::tryParseMessageFromBuffer(rxBuffer, 8, msg) == Protocol::Parser::ParseResult::OK) {
      if (msg.flag == Protocol::Constants::F_SYNC_S) {
        unsigned long t3 = msg.timestamp;
        int8_t diff = (int8_t)msg.data;

        unsigned long rtt = (t3 - t1) - (diff * 2);
        clientTime = t3 + rtt / 2;

        Serial.print("Time synchronized! New clientTime: ");
        Serial.println(clientTime);
      }
    } else {
      Serial.println("Failed to parse time sync response.");
    }
  } else {
    Serial.println("No time sync response received.");
  }
}
