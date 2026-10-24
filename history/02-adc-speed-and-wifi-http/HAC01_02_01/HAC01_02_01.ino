#define NUMBER 1000

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

}

void loop() {
  long milis1 = micros();
  for(int i = 0; i < NUMBER; i++){
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
    analogRead(A0);
  }
  long milis2 = micros();
  
  Serial.print(((double)(milis2-milis1)/(20 * NUMBER)));
  Serial.println("us");

  // put your main code here, to run repeatedly:
}
