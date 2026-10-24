#include <WiFiS3.h>
#include "ProtocolMessage.h"
#include "myLEDMatrix.h"
#include "BafferRead.h"

const char *ssid = "YOUR_SSID";  //YOUR_SSID
const char *password = "YOUR_PASSWORD";   //YOUR_PASSWORD
int status = WL_IDLE_STATUS;

IPAddress localIP(192, 168, 11, 90);
WiFiServer server(12345);  //こっちTCP
WiFiUDP udp;               //これがUDP

// 仮のLED状態（HIGHならON）
bool LED_state = false;
bool tcpConnect = false;

uint8_t time_buffer[8];

int32_t Time_Pressed_Button = 0;
//受付条件
bool accept = true;
unsigned long accept_start = 0;
constexpr unsigned long ACCEPT_MS = 500;

constexpr uint32_t DELAY_TIME = 1000;

uint8_t buffer[8];


void setup() {
  Serial.begin(115200);  //通信速度
  WiFi.config(localIP);

  initLEDMatrix();
  displayLED(LED_state);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  server.begin();
  udp.begin(6789);
}

void loop() {
  Protocol::ProtocolMessage msg;
  if (!accept && millis() - accept_start >= ACCEPT_MS) {
    accept = true;
  }
  // --- TCP処理 ---
  WiFiClient client = server.available();
  if (client) {
    if (readAndParseMessage(buffer, client, msg)) {
      // TCPでメッセージ正常受信
      Serial.println("TCP Message received");
      tcpConnect = true;
    } else {
      client.stop();
      Serial.println("TCP connection closed due to error");
    }
  }

  // --- UDP処理 ---
  if (!tcpConnect) {
    if (readAndParseMessageUDP(buffer, udp, msg)) {
      // UDPでメッセージ正常受信
      Serial.println("UDP Message received");
    }
  }

  uint32_t timeReceived = millis();

  auto type = Protocol::Parser::getMessageType(msg);  //LEDの状態取得

  if (type == Protocol::Parser::MessageType::LEDRequest) {
    Serial.println("Received LED state request");
    uint32_t chengeTime = Time_Pressed_Button + DELAY_TIME;

    // 応答生成
    uint8_t response[8];
    Protocol::Factory::createLEDControlResponse(response, chengeTime, LED_state);
    udp.beginPacket(udpState.remoteIP, udpState.remotePort);  // 相手のIPとポートに向けて送信開始
    udp.write(response, 8);                                   // データ書き込み
    udp.endPacket();                                          // 送信終了
    Serial.println("Response send");
    client.stop();
  }

  else if (type == Protocol::Parser::MessageType::ButtonPress) {
    Serial.println("Received ButtonPress");
    Time_Pressed_Button = msg.timestamp;

    if (accept) {              //正常な動作により通信された時の挙動
      LED_state = !LED_state;  //反転
      displayLED(LED_state);
      Serial.print("LED状態更新: ");
      Serial.println(LED_state);
      Serial.print(millis());
      Serial.println(" ms");

      accept = false;
      accept_start = millis();  // チャタリング防止開始

    } else {
      Serial.println("10ms以内の信号なので無視しました");
    }
    client.stop();
    //return;
  }

  if (type == Protocol::Parser::MessageType::TimeSyncRequest) {  //時間同期のリクエストが来た
    Serial.println("クライアントからのリクエストを受信");
    //通信遅延の計算
    uint32_t Server_clock = millis();
    Protocol::Parser::MessageType type = Protocol::Parser::getMessageType(msg);
    unsigned long Time = msg.timestamp;
    Serial.print("クライアントの時刻：");
    Serial.println(Time);                                                                             //クライアントの送信時の時刻を受け取る
    long diffTime = Server_clock - Time;                                                     //通信時間の計算
    if(diffTime < -256 || 256 < diffTime){
      Protocol::Factory::createTimeSyncResponse(time_buffer, millis(), 0);  //返信時間とサーバーとクライアントの時間差を送信//t3に時差、diffに返信時間を入れて送信
      //(Server_clock + 50) / 100)50ms切り上げて100ms単位で送信する->8bytesに収まる
      client.write(time_buffer, 8);
      Serial.println("誤差が256ms以上であったのでサーバーの時間を送信");
    }else{
      Protocol::Factory::createTimeSyncResponse(time_buffer, millis(), diffTime);
      client.write(time_buffer, 8);
      Serial.println("時間同期のレスポンスを送信");
    }
  } 

  /*
  else if (type == Protocol::Parser::MessageType::TimeSyncRequest) {
    uint32_t responseTime = timeReceived - msg.timestamp ;

    uint8_t response[8];
    Protocol::Factory::createTimeSyncResponse(response, millis(), responseTime);
    client.write(response, 8);
    client.stop();
    Serial.println("Response send");
  } 
  */

  tcpConnect = false;

  //delay(100);
}
