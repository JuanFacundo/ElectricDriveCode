int kp = 0.1;
float ki = 42;
float kd = 0.003;
float I = 0;
float last_e = 0;
float R = 0;
float y = 0;

void setup() {
  
  Serial.begin(9600);
}

void loop() {
  R = r(5);
  float e = R - y;
  y = PID(e);

  Serial.print(R);
  Serial.print(" ");
  Serial.println(y);
  delay(10);
}



//----------functions-------//
int r(int goTime){
  if (millis() >= goTime*1000){
    return 10;
  }
  else return 0;
}



float PID(int e){
  float P = e * kp;
  
  float D = ((e - last_e) / (10 * 0.001)) * (-kd);

  float I = ki*(I+e*10 * 0.001);

  float y = P + I + D;

  last_e = e;
  return y;
}
