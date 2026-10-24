#include <WiFiS3.h>
#include "ProtocolMessage.h"
#include "MyLEDMatrix.h"

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

const char* serverIP = "192.168.11.90";  // サーバーのIPアドレス
const uint16_t serverPort = 12345;       // サーバーのポート

WiFiClient client;

const int ledPin = A0;

const int buttonpin = 4;  // タクトスイッチ
bool lastButton = HIGH;

uint32_t lastSyncRequestTime = 0;
const uint32_t SYNC_INTERVAL = 10000;  // 10秒ごとに同期
int32_t adjustedTimeOffset = 0;
bool waitingForSyncResponse = false;
uint32_t syncRequestSentTime = 0;

void setup() {
  Serial.begin(115200);              //通信速度
  pinMode(buttonpin, INPUT_PULLUP);  // プルアップ構成
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected.");
  Serial.println(WiFi.localIP());

  pinMode(ledPin, OUTPUT);  // LEDピン初期化
}

void loop() {
  if (client.connect(serverIP, serverPort)) {
    Serial.println("Connected to server.");

    //時間同期の処理

    uint32_t now = millis();

    // --- ① 一定間隔で同期リクエストを送信 ---
    if (!waitingForSyncResponse && (now - lastSyncRequestTime >= SYNC_INTERVAL)) {
      syncRequestSentTime = millis();  // t1
      uint8_t buffer[8];
      Protocol::Factory::createTimeSyncRequest(buffer, syncRequestSentTime);
      client.write(buffer, 8);

      lastSyncRequestTime = now;
      waitingForSyncResponse = true;
    }

    // --- ② レスポンス受信チェック ---
    if (waitingForSyncResponse && client.available() >= 8) {
      uint8_t recvBuffer[8];
      client.readBytes(recvBuffer, 8);

      Protocol::ProtocolMessage msg;
      auto result = Protocol::Parser::tryParseMessageFromBuffer(recvBuffer, 8, msg);

      if (result == Protocol::Parser::ParseResult::OK && Protocol::Parser::getMessageType(msg) == Protocol::Parser::MessageType::TimeSyncResponse) {

        uint32_t t3 = millis();  // 受信時刻
        uint32_t t1 = msg.timestamp;
        uint8_t diff = msg.data;
        uint32_t t2 = t1 + diff;

        adjustedTimeOffset = ((int32_t)t2 + (int32_t)t3) / 2 - (int32_t)t3;

        Serial.print("Clock offset: ");
        Serial.println(adjustedTimeOffset);

        waitingForSyncResponse = false;  // 次の同期リクエストを許可
      }
    }

    //delay(40);

    //ボタン押下時の処理

    bool nowButton = digitalRead(buttonpin);

    // ボタン押下の立ち下がりを検出
    if (lastButton == HIGH && nowButton == LOW) {
      client.println("ボタンが押されました");

      uint8_t buffer[Protocol::Constants::PROTOCOL_MSG_SIZE];
      uint32_t currentTime = millis();
      Protocol::Factory::createButtonPressMessage(buffer, currentTime);

      // メッセージ送信（8バイト）
      client.write(buffer, Protocol::Constants::PROTOCOL_MSG_SIZE);

      // （オプション）レスポンス受信
      uint8_t response[Protocol::Constants::PROTOCOL_MSG_SIZE];
      size_t bytesRead = 0;
      unsigned long timeout = millis() + 200;

      while (client.connected() && millis() < timeout && bytesRead < sizeof(response)) {
        if (client.available()) {
          response[bytesRead++] = client.read();
        }
      }

      if (bytesRead == sizeof(response)) {
        Protocol::ProtocolMessage resMsg;
        auto result = Protocol::Parser::tryParseMessageFromBuffer(response, bytesRead, resMsg);

        if (result == Protocol::Parser::ParseResult::OK && Protocol::Parser::getMessageType(resMsg) == Protocol::Parser::MessageType::LEDResponse) {
          client.print("LED状態応答受信: ");
          client.println(resMsg.data == Protocol::Constants::LED_ON ? "ON" : "OFF");
        } else {
          client.println("応答メッセージ解析失敗または無効");
        }
      } else {
        client.println("応答なしまたは不完全");
      }

    }

    delay(40);//8バイト読み切る為通信が途中で終わったりして情報が混同し訳わからなくならないように

    lastButton = nowButton;


    // LED状態リクエストメッセージを生成
    uint8_t request[8];
    Protocol::Factory::createLEDStateRequest(request);
    client.write(request, 8);

    // 応答受信（8バイト）
    uint8_t response[8];
    size_t bytesRead = 0;
    unsigned long startTime = millis();
    while (bytesRead < 8 && (millis() - startTime) < 1000) {
      if (client.available()) {
        response[bytesRead++] = client.read();
      }
    }

    if (bytesRead == 8) {
      Protocol::ProtocolMessage msg;
      if (Protocol::Parser::tryParseMessageFromBuffer(response, 8, msg) == Protocol::Parser::ParseResult::OK) {
        if (Protocol::Parser::getMessageType(msg) == Protocol::Parser::MessageType::LEDResponse) {
          if (msg.data == Protocol::Constants::LED_ON) {
            uint32_t serverTime = millis() + adjustedTimeOffset;  //サーバの時計
            if (serverTime % 10000 < 1000) {
              digitalWrite(ledPin, HIGH);  // LEDをONにする
              Serial.println("LED is ON");
            }
          } else if (msg.data == Protocol::Constants::LED_OFF) {
            digitalWrite(ledPin, LOW);
            Serial.println("LED is OFF");
          } else {
            Serial.println("Unknown LED state data.");
          }
        } else {
          Serial.println("Unexpected message type.");
        }
      } else {
        Serial.println("Failed to parse response.");
      }
    } else {
      Serial.println("Timeout: did not receive full response.");
    }
    client.stop();
  }
  else {
    Serial.println("Connection failed.");
  }

  delay(69);  // 0.1秒おきにポーリング
}
