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
}

void loop() {
  int rpos = map(analogRead(A0),5,1023,0,500); //requested position
  //*
  if (rpos > pos){
    fullStep(0);
    pos++;
  } else if (rpos < pos){
    fullStep(1);
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

void fullStep(bool sgn){
  if (sgn){
    seq++;
  } else {
    seq = seq - 1;
  }
  if (seq == -1){
    seq = 3;
  }
  if (seq == 4){
    seq = 0;
  }
  if (seq == 0){
    digitalWrite(1,HIGH);
    digitalWrite(2,LOW);
    digitalWrite(3,LOW);
    digitalWrite(4,LOW);
  } else if (seq == 1){
    digitalWrite(1,LOW);
    digitalWrite(2,HIGH);
    digitalWrite(3,LOW);
    digitalWrite(4,LOW);
  } else if (seq == 2){
    digitalWrite(1,LOW);
    digitalWrite(2,LOW);
    digitalWrite(3,HIGH);
    digitalWrite(4,LOW);
  } else if (seq == 3){
    digitalWrite(1,LOW);
    digitalWrite(2,LOW);
    digitalWrite(3,LOW);
    digitalWrite(4,HIGH);
  }
}
