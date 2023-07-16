#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif

#define pot0 26
#define pot1 27
#define M 25
int memTl[M]; 
void setup() {
  Serial.begin(38400);
  for (int i=0;i<M;i++){
    memTl[i]=0;
  }
}

void loop() {
  for (int i=0;i<(M-1);i++){
    memTl[i] = memTl[i+1];
  }
  memTl[M-1] = analogRead(pot0);
  int medTl = 0;
  for (int i=0;i<M;i++){
    medTl = medTl + memTl[i];
  }
  medTl = medTl/M;
  Serial.print(analogRead(pot0));
  Serial.print(" ");
  Serial.println(medTl);
  //Serial.println(analogRead(pot1));
  delay(10);
}
