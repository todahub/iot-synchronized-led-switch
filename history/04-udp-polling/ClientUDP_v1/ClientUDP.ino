#include <WiFiS3.h>
#include "ProtocolMessage.h"
#include "MyLEDMatrix.h"
#include "BafferRead.h"
#include "antichata.h"

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

const char* serverIP = "192.168.11.90";  // サーバーのIPアドレス
const uint16_t serverPort = 12345;       // サーバーのポート
const uint16_t serverPortUDP = 6789;

WiFiClient client;
WiFiUDP udp;

const int ledPin = A0;

const int buttonpin = 4;  // タクトスイッチ
bool lastButton = HIGH;

uint32_t lastPolling = 0;
constexpr uint32_t POLLINGINTERVAL = 10;

AntiChata button(buttonpin); //アンチチャタリングシステム導入

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
  udp.begin(6789);

  pinMode(ledPin, OUTPUT);  // LEDピン初期化
}

void loop() {
  bool nowButton = digitalRead(buttonpin);
  static bool lastButtonState = HIGH;//アンチチャタリングシステム導入
  button.update();//アンチチャタリングシステム導入
  // ボタン押下の立ち下がりを検出
  if (button.wasPressed()) {//アンチチャタリングシステム導入
    Serial.println("ボタンが押されました");
    if(client.connect(serverIP, serverPort)){
      Serial.println("Connected to server.");

      uint8_t buffer[Protocol::Constants::PROTOCOL_MSG_SIZE];
      uint32_t currentTime = millis();
      Protocol::Factory::createButtonPressMessage(buffer, currentTime);

      // メッセージ送信（8バイト）
      client.write(buffer, Protocol::Constants::PROTOCOL_MSG_SIZE);
      Serial.println("送りました");
      // delay(100);
      // client.stop();
    }else {
      Serial.println("Failed to connect to server.");
    }
  }else {
    if (millis() - lastPolling >= POLLINGINTERVAL) {
      uint8_t request[8];
      Protocol::Factory::createLEDStateRequest(request);
      udp.beginPacket(serverIP, serverPortUDP);
      udp.write(request, 8);
      udp.endPacket();
      Serial.println("UDP送りました");
      lastPolling = millis();  // 応答が正常ならポーリング時間更新
      // 応答受信（8バイト）
      uint8_t response[8];
      Protocol::ProtocolMessage msg;
      if (readAndParseMessageUDP(response, udp, msg)) {

        if (Protocol::Parser::getMessageType(msg) == Protocol::Parser::MessageType::LEDResponse) {
          if (msg.data == Protocol::Constants::LED_ON) {
            digitalWrite(ledPin, HIGH);  // LEDをON
            Serial.println("LED is ON");
          } else if (msg.data == Protocol::Constants::LED_OFF) {
            digitalWrite(ledPin, LOW);   // LEDをOFF
            Serial.println("LED is OFF");
          } else {
            Serial.println("Unknown LED state data.");
          }
        } else {
          Serial.println("Unexpected message type.");
        }
      } else {
        Serial.println("Failed to read and parse UDP response.");
      }
    }
  }
  lastButton = nowButton;
  // Serial.println("loop終了");
  // delay(1000);
}

