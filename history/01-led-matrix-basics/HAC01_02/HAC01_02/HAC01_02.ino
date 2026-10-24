
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
byte clear[8][12] = {0};
void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.write((byte*)frame, 8 * 12);
  delay(500);
  Serial.write((byte*)clear, 8 * 12);
  delay(500);
}
