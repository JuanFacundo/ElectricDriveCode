float x[3] = {0,0,0};
float y[3] = {0,0,0};
float m[3] = {0,0,0};

void setup() {
  Serial.begin(38400);
}

void loop() {
  int pot1 = analogRead(27);
  int pot2 = analogRead(27);

  m[2] = m[1];
  m[1] = m[0];
  m[0] = pot1 + 1.556914*m[1] - 0.62659*m[2];
  y[0] = 0.01742*m[0] + 0.0348374*m[1] + 0.0174188*m[2];
  
  /*
  x[2] = x[1];
  x[1] = x[0];
  x[0] = (pot1 + pot1)/2.0;

  y[2] = y[1];
  y[1] = y[0];
  y[0] = 0.01742*x[0] + 0.0348374*x[1] + 0.0174188*x[2] + 1.556914*y[1] - 0.62659*y[2];
  //*/
  
  Serial.print(pot1);
  /*
  Serial.print(" ");
  Serial.print(440);
  Serial.print(" ");
  Serial.print(590);
  Serial.print(" ");
  Serial.print(908);
  Serial.print(" ");
  Serial.print(118);
  //*/
  /*
  Serial.print(" ");  //parámetros del shifter
  Serial.print(76);
  Serial.print(" ");
  Serial.print(230);
  Serial.print(" ");
  Serial.print(500);
  Serial.print(" ");
  Serial.print(905);
  Serial.print(" ");
  Serial.print(988);
  //*/ 
  Serial.print(" ");
  Serial.print(y[0]);
  Serial.print(" ");
  Serial.println(round(y[0]));
  
  delay(7);
}
