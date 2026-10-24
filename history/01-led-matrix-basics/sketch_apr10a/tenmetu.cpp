#include "tenmetu.h"

extern ArduinoLEDMatrix matrix; // 別の場所で定義された matrix を参照

tenmetu::tenmetu(int delayTime ,const byte frame[8][12])
  :delayTime(delayTime){ //引数で受け取った値をdelayTimeの初期化子に指定してる
    for(int i = 0; i < 8 ; i++)
      for(int j = 0; j < 12 ; j++){
        this->frame[i][j] = frame[i][j];//こっちはframeの初期化子に指定
      }
  }

void tenmetu::blinking(){
  matrix.renderBitmap(frame, 8, 12);
  delay(delayTime);
  matrix.clear();
  delay(delayTime); 
}
