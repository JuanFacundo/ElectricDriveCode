#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif

#define size 100

float x[size];


void setup() {
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);
  Serial.begin(38400);
  while(!Serial){
    delay(10);
  }
  for(int i=0;i<size;i++){
    x[i] = 0;
  }
}

void loop() {
  for(int i=0; i<size-1; i++){
    x[size-1-i] = x[size-2-i];
  }
  x[0] = 100*analogRead(A0)/1023;

  float y=0.0;
  for(int i=0; i<size; i++){
    y = y + x[i];
  }
  y = y/size;

  Serial.print("0 100 ");
  Serial.print(x[0]);
  Serial.print(" ");
  Serial.println(y);
  delay(10);

}
