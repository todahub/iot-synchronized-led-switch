#ifndef TENMETU_H
#define TENMETU_H

#include <Arduino.h>
#include "Arduino_LED_Matrix.h"

// 外部で定義された ArduinoLEDMatrix オブジェクトを利用
extern ArduinoLEDMatrix matrix;

class tenmetu {
private: //privateはこのクラス内でしか変更できない宣言
  byte frame[8][12];
  int delayTime;

public: //publicはクラス外から変更できる宣言
  tenmetu(int delayTime,const byte frame[8][12]); //コンストラクタと言ってオブジェクトが生成された時に実行される。返り値は返さないから引数指定はいらない

  void blinking();//
};

#endif // TENMETU_H