#include <WiFiS3.h>
#include "ProtocolMessage.h"
#include "MyLEDMatrix.h"
#include "BafferRead.h"
#include "antichata.h"

const char *ssid = "YOUR_SSID";
const char *password = "YOUR_PASSWORD";

const char *serverIP = "192.168.11.91";  // サーバーのIPアドレス
const uint16_t serverPort = 12345;       // サーバーのポート
const uint16_t serverPortUDP = 6789;

WiFiClient client;
WiFiUDP udp;

const int ledPin = A0;

const int buttonpin = 4;  // タクトスイッチ
bool lastButton = HIGH;

uint32_t i = 1;

uint8_t time_buffer[8];  // メッセージの送受信用の配列
int32_t diffTime = 0;
uint32_t Client_clock_base = 0;  // クライアントのサーバとの誤差の記録用
int32_t Client_millis_base = 0;  //一秒間計測用
int time_interval = 60000;       //1分間隔で時間同期
bool LEDChenge = false;

uint32_t lastPolling = 0;
constexpr uint32_t POLLINGINTERVAL = 10000;

AntiChata button(buttonpin);

void setup() {
  Serial.begin(9600);                //通信速度
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

  uint32_t i = 0;

  Protocol::ProtocolMessage msg;

  while (diffTime == 0) {  // クライアントの時間が同期されていないなら無限ループ
    if (client.connect(serverIP, serverPort)) {
      Serial.println("サーバーに接続");
      i++;
      Serial.print(i);
      Serial.print("回目の時間同期");
      // 時間同期リクエストを送信
      unsigned long NowTime1 = millis() + diffTime;                                // 時間同期リクエストを送った時刻
      Protocol::Factory::createTimeSyncRequest(time_buffer, NowTime1);  // 時間同期リクエストの送信
      client.write(time_buffer, 8);
      Serial.println("時間同期リクエストの送信");

      bool result = readAndParseMessage(time_buffer, client, msg);

      if (result) {
        // 時間同期リクエストのレスポンス処理
        // 最初の時間同期だから正確性に欠けるかも
        msg.fromBytes(time_buffer);
        uint32_t NowTimeSetup = millis() + diffTime;
        if(msg.data == 0) {
          diffTime = msg.timestamp - millis();
          Serial.print("クライアントとサーバの時間差");  // サーバのレスポンス(サーバ・クライアント間の時差)を受け取る
          Serial.println(diffTime);
          Serial.println(millis() + diffTime);
          //NowTime1 = millis(); //レスポンスを受け取った時刻
          //Client_clock_base = diffTime + (NowTime1 - msg.data * 1000) * 2;  //サーバ側で計算した時間差　ー　通信遅延　＊　２ (往復分)
          //Serial.print("クライアントとサーバの時間差-通信遅延*2");
          //Serial.println(Client_clock_base);
          Client_millis_base = millis();
          client.stop();
        } else {
          Serial.println(msg.data);
          diffTime = ((msg.timestamp - NowTimeSetup) + msg.data) / 2;
          Serial.println("時間同期成功！");
          Serial.print("サーバとの時間差");
          Serial.println(diffTime);
          client.stop();
        }  // 通信の遅延の計算はサーバとクライアントの内部時計の差が大きすぎるためなし
      } else {
        Serial.println("Connection failed.");
        client.stop();
        break;
      }
    }
  }
}

void loop() {
  bool nowButton = digitalRead(buttonpin);

  uint32_t nowTime = millis();
  static bool lastButtonState = HIGH;
  button.update();

  if (nowTime - Client_millis_base > time_interval) {  //1秒ごとに時間同期
    Serial.println(nowTime);
    Serial.println(Client_millis_base);
    Serial.println(time_interval);
    // 時間同期リクエストを送信
    if (client.connect(serverIP, serverPort)) {
      Serial.print("時間同期");
      Serial.print(i + 1);
      Serial.println("回目");
      i++;
      Protocol::ProtocolMessage msg;
      unsigned long NowTime2 = millis() + diffTime;                                // 時間同期リクエストを送った時刻
      Protocol::Factory::createTimeSyncRequest(time_buffer, NowTime2);  // 時間同期リクエストの送信
      client.write(time_buffer, 8);
      Serial.println("時間同期リクエストの送信");

      bool result = readAndParseMessage(time_buffer, client, msg);
      if (result) {
        uint32_t NowTime3 = millis() + diffTime;
        // 時間同期リクエストのレスポンス処理
        msg.fromBytes(time_buffer);
        //diffTime = msg.timestamp;  // サーバのレスポンス(サーバ・クライアント間の時差)を受け取る
        //uint32_t TimeLag = (Client_clock_base - msg.data * 1000) * 2;  // 通信遅延の計算
        //Client_clock_base = Client_clock_base - diffTime + TimeLag;    // 2回目以降の時間同期
        //Client_millis_base = millis();
        uint8_t offset = ((msg.timestamp - NowTime3) + msg.data) / 2;
        diffTime += offset;
        
        Serial.println("時間同期成功！");
        Serial.print("サーバとの時間差");
        Serial.println(diffTime);
        Serial.println(millis() + diffTime);
        client.stop();
      } else {
        Serial.println("時間同期失敗！");
        client.stop();
      }
      Client_millis_base = millis();
    }
  }
  // ボタン押下の立ち下がりを検出
  if (button.wasPressed()) {
    Serial.println("ボタンが押されました");
    uint32_t currentTime = millis() + diffTime;
    if (client.connect(serverIP, serverPort)) {
      Serial.println("Connected to server.");
      uint8_t buffer[Protocol::Constants::PROTOCOL_MSG_SIZE];

      Protocol::Factory::createButtonPressMessage(buffer, currentTime);

      // メッセージ送信（8バイト）
      client.write(buffer, Protocol::Constants::PROTOCOL_MSG_SIZE);
      Serial.println("送りました");
      // delay(100);
      // client.stop();
      //uint32_t Client_clock_base = millis();  //ボタンが押された時の時間
    } else {
      Serial.println("Failed to connect to server.");
    }
  } else {
    if (millis() - lastPolling >= POLLINGINTERVAL) {
      uint8_t request[8];
      Protocol::Factory::createLEDStateRequest(request);
      udp.beginPacket(serverIP, serverPortUDP);
      udp.write(request, 8);
      udp.endPacket();
      Serial.println("UDP送りました");
      //Serial.println(Client_millis_base);
      //Serial.println(nowTime);

      lastPolling = millis();  // 応答が正常ならポーリング時間更新
      // 応答受信（8バイト）
      uint8_t response[8];
      Protocol::ProtocolMessage msg;
      if (readAndParseMessageUDP(response, udp, msg)) {

        if (Protocol::Parser::getMessageType(msg) == Protocol::Parser::MessageType::LEDResponse) {
          /*
          Serial.print("時間差:");
          Serial.println(Client_clock_base);
          Serial.print("現在時刻");
          Serial.println(millis());
          */

          //uint32_t Client_clock_base = millis();
          if (msg.timestamp != 0) {
            Serial.println(msg.timestamp);
            Client_clock_base = msg.timestamp;
            LEDChenge = true;
          } /*else {
            if()
          }*/

        } else {
          Serial.println("Failed to read and parse UDP response.");
        }
      }
      if (LEDChenge && Client_clock_base <= millis() + diffTime) {  //ボタンが押された時間から決められた時間経ったら
        Serial.println(msg.timestamp);
        ///*
        Serial.println(msg.timestamp);
        Serial.println(Client_clock_base);
        Serial.println(millis());
        LEDChenge = false;
        //*/
        if (msg.data == Protocol::Constants::LED_ON) {
          digitalWrite(ledPin, HIGH);  // LEDをON
          Serial.print("実際のピン状態: ");
          Serial.println(digitalRead(ledPin));
          Serial.println("LED is ON");
        } else if (msg.data == Protocol::Constants::LED_OFF) {
          digitalWrite(ledPin, LOW);  // LEDをOFF
          Serial.println("LED is OFF");
        } else {
          Serial.println("Unknown LED state data.");
        }
      }
    } else {
      //Serial.println("LED is not chenge.");
    }
  }

  //*

  //*/

  lastButton = nowButton;
  // Serial.println("loop終了");
  //delay(1000);
}
