#include <Servo.h>

Servo myservo;  // create servo object to control a servo
int req;

void setup() {
  myservo.attach(16);  // attaches the servo on pin 9 to the servo object
  myservo.write(0);
}

void loop() {    // reads the value of the potentiometer (value between 0 and 1023)
  int valu = map(analogRead(A0), 0, 1023, 0, 180); 
  if (valu>req){
    req++;
  } else if (valu<req){
    req-=1;
  }
  myservo.write(req);                  // sets the servo position according to the scaled value
  delay(1);                           // waits for the servo to get there
}
