#include "Arduino_LED_Matrix.h"
ArduinoLEDMatrix matrix;
byte frame[8][12] = {
  { 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0 },
  { 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0 },
  { 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

void setup() {
  matrix.begin();
  // put your setup code here, to run once:

}

void loop() {
  matrix.renderBitmap(frame, 8, 12);
  delay(500);
  matrix.clear();
  delay(500);

  // put your main code here, to run repeatedly:
}
