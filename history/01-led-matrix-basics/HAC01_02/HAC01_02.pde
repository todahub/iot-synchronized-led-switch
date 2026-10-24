import processing.serial.*;
Serial port;// シリアルポート
byte[] data;
void setup(){
// ウィンドウサイズ
size(240,160);
// ポートを初期化
port = new Serial(this, "/dev/cu.usbmodem4827E2E0EC882",115200);
// シリアルポートの初期化
port.clear();
port.buffer(8 * 12);
}
void draw(){
// 背景色は白
background(255);
// dataの中身がnull出ないか確かめる
if(data != null)
drawData(data);
}

// データが送信されてきたら呼び出される
void serialEvent(Serial p){
// ポートからデータを取得
data = p.readBytes();
// 書き出して値を確認
println("Received " + data.length + " bytes");
}

void drawData(byte data[]){
  int size = 20;
  for(int i = 0; i < 8; i++)
    for(int j = 0; j < 12;j++){
      int idx = i * 12 + j;
     if(data[idx] == 1)
       fill(0);
     else 
       fill(255);
     noStroke();//境界線を消す
     rect(j * size, i * size, size, size);
    }
}
