#include "antichata.h"

AntiChata::AntiChata(int buttonPin, unsigned long debounceDelay)
  : buttonPin(buttonPin),
    debounceDelay(debounceDelay),
    buttonState(HIGH),
    lastStableState(HIGH),
    lastDebounceTime(0),
    pressedFlag(false),
    pressEvent(false),
    releaseEvent(false)
{
  pinMode(buttonPin, INPUT_PULLUP);
}

void AntiChata::update() {
  int reading = digitalRead(buttonPin);

  if (reading != lastStableState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW && !pressedFlag) {
        pressedFlag = true;
        pressEvent = true;
        releaseEvent = false;
      } else if (buttonState == HIGH && pressedFlag) {
        pressedFlag = false;
        pressEvent = false;
        releaseEvent = true;
      }
    }
  }

  lastStableState = reading;
}

bool AntiChata::wasPressed() {
  bool result = pressEvent;
  pressEvent = false;
  return result;
}

bool AntiChata::wasReleased() {
  bool result = releaseEvent;
  releaseEvent = false;
  return result;
}
