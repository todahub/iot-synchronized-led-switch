#include<WiFiS3.h>

char ssid[] = "YOUR_SSID";
char pass [] = "YOUR_PASSWORD";
IPAddress local_ip(192,168,11,100);
int status = WL_IDLE_STATUS;
WiFiServer server(80);

void setup(){
  Serial.begin(9600);

  WiFi.config(local_ip);
  server.begin();
  // statusがWL_CONNECTEDになるまでwhileを繰り返す
  while(status != WL_CONNECTED){
    //WPA2のネットワークに接続:
    status = WiFi.begin(ssid, pass);
    // 10秒待つ：
    delay (10000);
  }
  Serial.println("ルーターに接続できました！");

  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
}

void loop() {
  WiFiClient client = server.available();

  if(client){
    Serial.println("クライアントとの接続開始！");
    while(client.connected()){
      String currentLine = "";
      if(client.available()){
        char c = client.read();
        Serial.write(c);
        if(c =='\n'){
          if(currentLine.length() == 0){
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print("<p>Hello!!</p>");
            client.println();
            break;
          }else{
            currentLine = "";
          }
        }else if(c != '\r'){
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("クライアントとの接続終了!");
  }
}
