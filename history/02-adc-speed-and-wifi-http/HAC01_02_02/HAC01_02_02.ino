#define DIVISION 0b000
#define NUMBER 1000

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  R_SYSTEM->PRCR = 0xA501;
  R_SYSTEM->SCKDIVCR_b.PCKC = DIVISION;
  R_SYSTEM->PRCR = 0xA500;

  Serial.println(R_SYSTEM->SCKDIVCR_b.PCKC,BIN);
  
}

void loop() {
  speedtest();

  while(true);
  
  // put your main code here, to run repeatedly:

}

void speedtest(){
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
  
  Serial.print("分周比");
  Serial.print(DIVISION);
  Serial.print(":");
  Serial.print(((double)(milis2-milis1)/(20 * NUMBER)));
  Serial.println("us");
}
