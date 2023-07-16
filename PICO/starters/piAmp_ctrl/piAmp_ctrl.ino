#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif


float m[3] = {0,0,0};
float holdTorq = 0;
bool flag = 0;

void setup() {
  Serial.begin(38400);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  int pot0 = analogRead(26);
  float ask = (200.0 / 341.0) * pot0;

  int pot1 = analogRead(27);
  m[2] = m[1];
  m[1] = m[0];
  m[0] = pot1 + 1.556914*m[1] - 0.62659*m[2];
  float DCL = (200.0 / 341.0) * (0.01742*m[0] + 0.0348374*m[1] + 0.0174188*m[2]);

  float out = 0;

  if (ask > DCL){
    if (flag){
      if (holdTorq > (DCL - 2)){
        holdTorq -= 5;
      } else if (holdTorq < (DCL - 7)){
        holdTorq++;
      }
    } else {
      holdTorq = ask - 1;
    }
    flag = 1;
    out = holdTorq;
  } else {
    if (flag){
      if(holdTorq < (ask-0.5)){
        holdTorq++;
      } else {
        flag = 0;
      }
    }
    
    
  }

  if (flag){
    out = holdTorq;
  } else {
    out = ask;
  }
  
  
  Serial.print(0);
  Serial.print(" ");
  Serial.print(600);
  Serial.print(" ");
  Serial.print(out);
  Serial.print(" ");
  Serial.print(ask);
  Serial.print(" ");
  Serial.println(DCL);
  delay(20);
}
