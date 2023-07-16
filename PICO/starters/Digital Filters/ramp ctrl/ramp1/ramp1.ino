#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif

#define ramp 1.5
float y=0;


void setup() {
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);
  Serial.begin(38400);
  while(!Serial){
    delay(10);
  }
}

int lastT = millis();

void loop() {
  float x = 100*analogRead(A0)/1023;
  float D = y-x;
  if(abs(D)>ramp){
    if(D<0){
      y+=ramp*10/(millis()-lastT);
    } else {
      y-=ramp*10/(millis()-lastT);
    }
  } else if(D!=0){
    y=x;
  }
  lastT = millis();
  Serial.print("0 100 ");
  Serial.print(x);
  Serial.print(" ");
  Serial.println(y);
  delay(10);
}
