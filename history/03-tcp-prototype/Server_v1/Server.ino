#include <WiFiS3.h>
#include "ProtocolMessage.h"
#include "myLEDMatrix.h"

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
int status = WL_IDLE_STATUS;

IPAddress localIP(192, 168, 11, 89); 
WiFiServer server(12345);

// 仮のLED状態（HIGHならON）
bool ledState = false;

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

  while (bytesRead < 8) {
    if (client.available()) {
      buffer[bytesRead++] = client.read();
    }
  }

  uint32_t timeReceived;

  Protocol::ProtocolMessage msg;
  if (Protocol::Parser::tryParseMessageFromBuffer(buffer, 8, msg) != Protocol::Parser::ParseResult::OK) {
    Serial.println("CRC error or bad message");
    timeReceived = millis();
    client.stop();
    return;
  }

  auto type = Protocol::Parser::getMessageType(msg);
  if (type == Protocol::Parser::MessageType::LEDRequest) {
    Serial.println("Received LED state request");
    uint32_t chengeTime = msg.timestamp + DELAY_TIME;

    // 応答生成
    uint8_t response[8];
    Protocol::Factory::createLEDControlResponse(response, chengeTime, LED_state);
    client.write(response, 8);
  }

  else if (type == Protocol::Parser::MessageType::ButtonPress) {
    Serial.println("Received ButtonPress");
       
    if (!accept && millis() - accept_start > ACCEPT_MS) {
    accept = true;
    }

    if (accept) {
        LED_state = !LED_state;
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
    uint32_t responseTime = msg.timestamp - timeReceived;

    uint8_t response[8];
    Protocol::Factory::createTimeSyncResponse(response, millis(), responseTime);
    client.write(response, 8);
  }
}
