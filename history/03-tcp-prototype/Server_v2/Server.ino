#include <WiFiS3.h>
#include "ProtocolMessage.h"
#include "myLEDMatrix.h"
#include "BafferRead.h"

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
int status = WL_IDLE_STATUS;

IPAddress localIP(192, 168, 11, 89); 
WiFiServer server(12345);

// 仮のLED状態（HIGHならON）
bool LED_state = false;

//受付条件
bool accept = true;
unsigned long accept_start = 0;
constexpr unsigned long ACCEPT_MS = 10;

constexpr uint32_t DELAY_TIME = 1000;

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
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  uint8_t buffer[8];
  size_t bytesRead = 0;
  unsigned long start = millis();
  
  if (!accept && millis() - accept_start >= ACCEPT_MS) {
    accept = true;
  }

  Protocol::ProtocolMessage msg; //エラー処理
  bool bufferRead = readAndParseMessage(buffer,client,msg);
  if(!bufferRead){
    client.stop();
    Serial.println("Connection closed due to error.");
    return;
  }else{
    client.stop();
    Serial.println("Connection was closed correctly");
    return;
  }
  uint32_t timeReceived = millis();

  auto type = Protocol::Parser::getMessageType(msg); //LEDの状態取得

  if (type == Protocol::Parser::MessageType::LEDRequest) {
    Serial.println("Received LED state request");
    uint32_t chengeTime = msg.timestamp + DELAY_TIME;

    // 応答生成
    uint8_t response[8];
    Protocol::Factory::createLEDControlResponse(response, chengeTime, LED_state);
    client.write(response, 8);
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
    } else {
      Serial.println("10ms以内の信号なので無視しました");
    }
  }
  

  else if (type == Protocol::Parser::MessageType::TimeSyncRequest) {
    uint32_t responseTime = timeReceived - msg.timestamp ;

    uint8_t response[8];
    Protocol::Factory::createTimeSyncResponse(response, millis(), responseTime);
    client.write(response, 8);
    Serial.println("Response send");
  }
}
