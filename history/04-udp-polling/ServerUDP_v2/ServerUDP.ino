#include <WiFiS3.h>
#include "ProtocolMessage.h"
#include "myLEDMatrix.h"
#include "BafferRead.h"

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
int status = WL_IDLE_STATUS;

IPAddress localIP(192, 168, 11, 90); 
WiFiServer server(12345);//こっちTCP
WiFiUDP udp;//これがUDP

// 仮のLED状態（HIGHならON）
bool LED_state = false;
bool tcpConnect = false;

//受付条件
bool accept = true;
unsigned long accept_start = 0;
constexpr unsigned long ACCEPT_MS = 10;

constexpr uint32_t DELAY_TIME = 1000;

uint8_t buffer[8];
Protocol::ProtocolMessage msg;

void setup() {
  Serial.begin(115200);//通信速度
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
  if(!tcpConnect){
    if (readAndParseMessageUDP(buffer, udp, msg)) {
      // UDPでメッセージ正常受信
      Serial.println("UDP Message received");
    }
  }

  uint32_t timeReceived = millis();


  auto type = Protocol::Parser::getMessageType(msg); //LEDの状態取得

  if (type == Protocol::Parser::MessageType::LEDRequest) {
    Serial.println("Received LED state request");
    uint32_t chengeTime = msg.timestamp + DELAY_TIME;

    // 応答生成
    uint8_t response[8];
    Protocol::Factory::createLEDControlResponse(response, chengeTime, LED_state);
    udp.beginPacket(udpState.remoteIP, udpState.remotePort);// 相手のIPとポートに向けて送信開始
    udp.write(response, 8);                             // データ書き込み
    udp.endPacket();                                   // 送信終了
    Serial.println("Response send");
  }

  else if (type == Protocol::Parser::MessageType::ButtonPress) {
    Serial.println("Received ButtonPress");

    if (accept) { //正常な動作により通信された時の挙動
        LED_state = !LED_state; //反転
        displayLED(LED_state);
        Serial.print("LED状態更新: ");
        Serial.println(LED_state);
        Serial.print(millis());
        Serial.println(" ms");

        accept = false;
        accept_start = millis();  // チャタリング防止開始
        tcpConnect = false;
        return;
        
    } else {
      Serial.println("10ms以内の信号なので無視しました");
    }
    while (client.available()) {
      client.read();  // バッファを空にする
    }
    client.stop();
  }
  

  else if (type == Protocol::Parser::MessageType::TimeSyncRequest) {
    uint32_t responseTime = timeReceived - msg.timestamp ;

    uint8_t response[8];
    Protocol::Factory::createTimeSyncResponse(response, millis(), responseTime);
    client.write(response, 8);
    client.stop();
    Serial.println("Response send");
    tcpConnect = false;
  } 
  
  
}
