#include<WiFiS3.h>

#define SERVER "192.168.11.100"


char ssid[] = "YOUR_SSID";
char pass [] = "YOUR_PASSWORD";
int status = WL_IDLE_STATUS;

char server[] = SERVER;


WiFiClient client;

void setup(){
  Serial.begin(9600);

  // statusがWL_CONNECTEDになるまでwhileを繰り返す
  while(status != WL_CONNECTED){
    //WPA2のネットワークに接続:
    status = WiFi.begin(ssid, pass);
    // 10秒待つ：
    delay (10000);
  }
  Serial.println("ルーターに接続できました！");

  IPAddress ip = WiFi.localIP();
  Serial.print("IPアドレス:");
  Serial.println(ip);


  printWifiStatus();

  if (client.connect(server, 80)) {
    Serial.println("サーバーに接続しました");
    // Make a HTTP request:
    client.println("GET /HTTP/1.1");
    client.print("Host: ");
    client.println(SERVER);
    client.println("Connection: close");
    client.println();
  }
}

void read_response() {
/* -------------------------------------------------------------------------- */  
  uint32_t received_data_num = 0;
  while (client.available()) {
    /* actual data reception */
    char c = client.read();
    /* print data to serial port */
    Serial.print(c);
    /* wrap data to 80 columns*/
    received_data_num++;
    if(received_data_num % 80 == 0) { 
      Serial.println();
    }
  }  
}

void loop() {
  read_response();

  if (!client.connected()) {
    Serial.println();
    Serial.println("サーバーから切断されました");
    client.stop();

    // do nothing forevermore:
    while (true);
  }

}

void printWifiStatus() {
/* -------------------------------------------------------------------------- */  
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}
