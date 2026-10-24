#include <Arduino.h>
#include "Arduino_LED_Matrix.h"
#include "tenmetu.h"
#define INTERVAL 50 //myTenmetuの第一引数に使う。delayの時間簡単にいじれる
ArduinoLEDMatrix matrix; //オブジェクト
byte pattern[8][12] = {
  { 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0 },
  { 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0 },
  { 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
tenmetu myTenmetu(INTERVAL,pattern); //オブジェクトとコンストラクタの引数指定

void setup() {
  matrix.begin();
  // put your setup code here, to run once:

}

void loop() {
  myTenmetu.blinking();
  // put your main code here, to run repeatedly:
}
