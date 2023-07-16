#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif

long goTime;
float integ = 0;
float x = 0;
float y = 0;
float last_error = 0;

float kp = 0.005;
float ki = 50;
float kd = 0;

void setup() {
  Serial.begin(38400);
  while(!Serial){
    delay(10);
  }
  goTime = millis() + 3000;

}

void loop() {
  if(millis() > goTime){
    x = 1.0;
  }
  Tfilter(10);
  Serial.print("0 ");
  Serial.print(x);
  Serial.print(" ");
  Serial.println(y);

  delay(10);
  
}




void Tfilter(long dt){
  if (1){
    long error = x - y;

    float P = error * kp;

    integ += error * dt*0.001;
    float I = integ * ki;

    float D = ((error - last_error) / (dt*0.001))*(-kd);

    y = (P + I + D);

    last_error = error;
  }
}