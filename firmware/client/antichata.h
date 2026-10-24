#ifndef ANTICHATA_H
#define ANTICHATA_H

#include <Arduino.h>

class AntiChata {
public:
  AntiChata(int buttonPin, unsigned long debounceDelay = 10);
  void update();
  bool wasPressed();  // 押下イベント（LOW→検出）
  bool wasReleased(); // 離上イベント（HIGH→検出）

private:
  int buttonPin;
  unsigned long debounceDelay;

  int buttonState;
  int lastStableState;
  unsigned long lastDebounceTime;

  bool pressedFlag;
  bool pressEvent;
  bool releaseEvent;
};

#endif
