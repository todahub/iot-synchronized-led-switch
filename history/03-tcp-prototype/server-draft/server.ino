#include <WiFiS3.h>
#include <WiFiUdp.h>

// --- Wi-Fi設定 ---
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// --- サーバー設定 ---
IPAddress serverIP(192, 168, 1, 100);
const unsigned int serverPort = 12345;

// --- UDPオブジェクト ---
WiFiUDP udp;

// --- メッセージバッファ ---
uint8_t message[8];

// CRC-16-CCITT用の関数（MSBファースト）
uint16_t crc16_ccitt(const uint8_t* data, size_t length) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < length; ++i) {
    crc ^= (uint16_t)data[i] << 8;
    for (int j = 0; j < 8; ++j) {
      if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
      else crc <<= 1;
    }
  }
  return crc & 0xFFFF;
}

// ボタン押下通知メッセージを送信
void sendButtonPress() {
  uint32_t timestamp = millis();

  message[0] = 0x01; // フラグ
  message[1] = (timestamp >> 24) & 0xFF;
  message[2] = (timestamp >> 16) & 0xFF;
  message[3] = (timestamp >> 8) & 0xFF;
  message[4] = timestamp & 0xFF;
  message[5] = 0x00; // データ未使用

  uint16_t crc = crc16_ccitt(message, 6); // 前6バイトのみCRC対象
  message[6] = (crc >> 8) & 0xFF; // MSB先
  message[7] = crc & 0xFF;

  udp.beginPacket(serverIP, serverPort);
  udp.write(message, 8);
  udp.endPacket();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");

  udp.begin(12346); // クライアント側ポート（任意）
}

void loop() {
  if (digitalRead(2) == LOW) { // ピン2に接続されたボタンが押されたら送信
    sendButtonPress();
    delay(500); // デバウンス
  }

  // ここに受信処理を実装可
}
