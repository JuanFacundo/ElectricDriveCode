//mirandolo desde atras es negro rojo del A, rojo negro del B
/*

gpio1 to in1
gpio2 to in3
gpio3 to in2
gpio4 to in4
 
*/

#define sped 2
int i = 0;
int sign = 0;
int pos = 0;
int seq = 0;

void setup() {
  Serial.begin(38400);
  pinMode(1,OUTPUT);
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);

  //set to 0
  for (int i=0;i<400;i++){
    softStep(0);
  }
}

void loop() {
  int rpos = map(analogRead(A0),5,1023,0,500); //requested position
  //*
  if (rpos > pos){
    halfStep(1);
    pos++;
  } else if (rpos < pos){
    halfStep(0);
    pos = pos-1;
  }
  Serial.println(seq);
  delay(sped);
  //*/
  //Serial.println(rpos);
  
  
  /*
  if (i == 0){
    sign = 1;
  } else if (i == 30){
    sign = 0;
  }

  softStep(sign);
  i = i + ((sign - (1.0/2.0))*2);
  Serial.println(sign);
  //*/
}




void halfStep(bool sgn){
  if (sgn){
    seq++;
  } else {
    seq = seq - 1;
  }
  if (seq == -1){
    seq = 7;
  }
  if (seq == 8){
    seq = 0;
  }
  if (seq == 0){
    digitalWrite(1,LOW);
    digitalWrite(2,LOW);
    digitalWrite(3,LOW);
    digitalWrite(4,HIGH);
  } else if (seq == 1){
    digitalWrite(1,HIGH);
      digitalWrite(2,LOW);
      digitalWrite(3,LOW);
      digitalWrite(4,HIGH);
  } else if (seq == 2){
    digitalWrite(1,HIGH);
      digitalWrite(2,LOW);
      digitalWrite(3,LOW);
      digitalWrite(4,LOW);
  } else if (seq == 3){
    digitalWrite(1,HIGH);
      digitalWrite(2,HIGH);
      digitalWrite(3,LOW);
      digitalWrite(4,LOW);
  } else if (seq == 4){
    digitalWrite(1,LOW);
      digitalWrite(2,HIGH);
      digitalWrite(3,LOW);
      digitalWrite(4,LOW);
  } else if (seq == 5){
    digitalWrite(1,LOW);
      digitalWrite(2,HIGH);
      digitalWrite(3,HIGH);
      digitalWrite(4,LOW);
  } else if (seq == 6){
    digitalWrite(1,LOW);
      digitalWrite(2,LOW);
      digitalWrite(3,HIGH);
      digitalWrite(4,LOW);
  } else if (seq == 7){
    digitalWrite(1,LOW);
      digitalWrite(2,LOW);
      digitalWrite(3,HIGH);
      digitalWrite(4,HIGH);
  }
    
      
}


void softStep(bool sgn){  //sgn = 1: turning clockwise
  if (sgn){
    digitalWrite(1,HIGH);
    digitalWrite(2,LOW);
    digitalWrite(3,LOW);
    digitalWrite(4,LOW);
    delay(sped);
    digitalWrite(1,HIGH);
    digitalWrite(2,HIGH);
    digitalWrite(3,LOW);
    digitalWrite(4,LOW);
    delay(sped);
    
    digitalWrite(1,LOW);
    digitalWrite(2,HIGH);
    digitalWrite(3,LOW);
    digitalWrite(4,LOW);
    delay(sped);
    digitalWrite(1,LOW);
    digitalWrite(2,HIGH);
    digitalWrite(3,HIGH);
    digitalWrite(4,LOW);
    delay(sped);
    
    digitalWrite(1,LOW);
    digitalWrite(2,LOW);
    digitalWrite(3,HIGH);
    digitalWrite(4,LOW);
    delay(sped);
    digitalWrite(1,LOW);
    digitalWrite(2,LOW);
    digitalWrite(3,HIGH);
    digitalWrite(4,HIGH);
    delay(sped);
    
    digitalWrite(1,LOW);
    digitalWrite(2,LOW);
    digitalWrite(3,LOW);
    digitalWrite(4,HIGH);
    delay(sped);
    digitalWrite(1,HIGH);
    digitalWrite(2,LOW);
    digitalWrite(3,LOW);
    digitalWrite(4,HIGH);
    delay(sped);
    
  } else {
    digitalWrite(1,LOW);
    digitalWrite(2,LOW);
    digitalWrite(3,LOW);
    digitalWrite(4,HIGH);
    delay(sped);
    digitalWrite(1,LOW);
    digitalWrite(2,LOW);
    digitalWrite(3,HIGH);
    digitalWrite(4,HIGH);
    delay(sped);

    digitalWrite(1,LOW);
    digitalWrite(2,LOW);
    digitalWrite(3,HIGH);
    digitalWrite(4,LOW);
    delay(sped);
    digitalWrite(1,LOW);
    digitalWrite(2,HIGH);
    digitalWrite(3,HIGH);
    digitalWrite(4,LOW);
    delay(sped);
    
    digitalWrite(1,LOW);
    digitalWrite(2,HIGH);
    digitalWrite(3,LOW);
    digitalWrite(4,LOW);
    delay(sped);
    digitalWrite(1,HIGH);
    digitalWrite(2,HIGH);
    digitalWrite(3,LOW);
    digitalWrite(4,LOW);
    delay(sped);

    digitalWrite(1,HIGH);
    digitalWrite(2,LOW);
    digitalWrite(3,LOW);
    digitalWrite(4,LOW);
    delay(sped);
    digitalWrite(1,HIGH);
    digitalWrite(2,LOW);
    digitalWrite(3,LOW);
    digitalWrite(4,HIGH);
    delay(sped);
  }
}
