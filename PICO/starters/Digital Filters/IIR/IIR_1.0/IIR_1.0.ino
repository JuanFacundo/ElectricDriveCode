float m[3] = {0,0,0};

void setup() {
  Serial.begin(38400);
}

void loop() {
  int pot1 = analogRead(27);

  m[2] = m[1];
  m[1] = m[0];
  m[0] = pot1 + 1.556914*m[1] - 0.62659*m[2];
  float y = 0.01742*m[0] + 0.0348374*m[1] + 0.0174188*m[2];

  Serial.println(y);

  delay(7);
}
