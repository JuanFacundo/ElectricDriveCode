#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif


float m[3] = {0,0,0};
float a0 = 3.9227e-05;
float a1 = 2*a0;
float a2 = a0;

float b1 = -1.9833;
float b2 = 0.9912;
int x = 0;


void setup() {
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);
  Serial.begin(38400);
  while(!Serial){
    delay(10);
  }

}

void loop() {
  x = analogRead(A0);

  m[2] = m[1];
  m[1] = m[0];
  m[0] = x - b1*m[1] - b2*m[2];
  float y = (a0*m[0] + a1*m[1] + a2*m[2]) * 49.98973;

  Serial.print(x);
  Serial.print(" ");
  Serial.println((int) y);
  delay(10);
}
