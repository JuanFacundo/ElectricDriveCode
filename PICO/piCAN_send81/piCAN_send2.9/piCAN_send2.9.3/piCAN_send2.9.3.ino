#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif

//—————LIBS——————————————————————————————————————————————————————————————
#include <ACAN2515.h>


//--------pines mcp--------//
//static const byte MCP2515_INT  = 1; // INT output of MCP2515  //MCP2515 CON INT
static const byte MCP2515_INT = 255; // INT output of MCP2515  //MCP2515 SIN INT
static const byte MCP2515_SCK = 2;  // SCK input of MCP2515
static const byte MCP2515_MOSI = 3; // SDI input of MCP2515
static const byte MCP2515_MISO = 4; // SDO output of MCP2515
static const byte MCP2515_CS = 5;   // CS input of MCP2515
#define Rpin 6
#define Fpin 7
#define startRel 8
#define startBut 10
#define potT 26
#define potS 27
#define shiftPin 28

//--------variables CAN------//
ACAN2515 can(MCP2515_CS, SPI, MCP2515_INT);
CANMessage canMsg;
static const uint32_t QUARTZ_FREQUENCY = 8UL * 1000UL * 1000UL; // 8 MHz //  MCP2515 Quartz: adaptado al diseño

//--------variables programa--------//
char gears[] = {'N','R','D','P','S','f','X'};
const char* modes[] = {"Initialization",
                 "Standby",
                 "TorqueCtrl",
                 "SpeedCtrl",
                 "Discharge",
                 "Reserved",
                 "!EMERGENCY SHUTDOWN!",
                 "Allow12VBattPowerOff",
                 "SystemFault", "SelfCheckField"};
const char* states[] = {"pow consum","pow gen","shutdown","READY"};
int phy_rpm=0;
byte msgCount=0;
byte shiftToll = 10;
byte msgEna = 1;
int memTl[25];
int memSH[10];
bool onoff = 0;
bool StartStopPress = 0;
long lastSend = millis();
long lastRead = millis();



//-------------SETUP----------//
void setup() {
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);
  
  setupSensors();
  
  Serial.begin(38400);
  while (!Serial){
    delay(1);
  }
  
  setupCAN();
  delay(300);
}








void loop() {  //-----------------------LOOP
  can.poll();
  StartStop();
  if (onoff){
    if (millis() >= (lastRead + 7)){
      byte shiftNow = readShifter();
      byte shiftWant = readGear();
      
      if (shiftNow != shiftWant){
        shifter(shiftNow,shiftWant);
        shiftToll = 0;
      }
      else{
        digitalWrite(Rpin,HIGH); //stop shift movement
        digitalWrite(Fpin,HIGH); //stop shift movement
        hardread();
      }
    }
    if (millis() >= (lastSend + 10)){
      sender();
    }
  }
  
  delay(1);
}












//---------------------------------------------------------------------------FUNCTIONS----------------------------------------------------------------------------//

//---------------------------------settings--------------------------------//
void setupCAN(){
  SPI.setSCK(MCP2515_SCK);
  SPI.setTX(MCP2515_MOSI);
  SPI.setRX(MCP2515_MISO);
  SPI.setCS(MCP2515_CS);
  //--- Begin SPI
  SPI.begin();
  //--- Configure ACAN2515
  ACAN2515Settings settings(QUARTZ_FREQUENCY, 500UL * 1000UL); // CAN bit rate 500 kb/s
  settings.mRequestedMode = ACAN2515Settings::NormalMode;      // NormalMode (default value), ListenOnlyMode,LoopBackMode, SleepMode.
  //settings.mTransmitBuffer0Size = 30 ;
  //const uint16_t errorCode = can.begin(settings, [] {can.isr();}); //MCP2515 CON INT
  const uint16_t errorCode = can.begin(settings, NULL); // MCP2515 SIN INT

  if (errorCode == 0) {
    Serial.print ("Bit Rate prescaler: ") ;
    Serial.println (settings.mBitRatePrescaler) ;
    Serial.print ("Propagation Segment: ") ;
    Serial.println (settings.mPropagationSegment) ;
    Serial.print ("Phase segment 1: ") ;
    Serial.println (settings.mPhaseSegment1) ;
    Serial.print ("Phase segment 2: ") ;
    Serial.println (settings.mPhaseSegment2) ;
    Serial.print ("SJW: ") ;
    Serial.println (settings.mSJW) ;
    Serial.print ("Triple Sampling: ") ;
    Serial.println (settings.mTripleSampling ? "yes" : "no") ;
    Serial.print ("Actual bit rate: ") ;
    Serial.print (settings.actualBitRate ()) ;
    Serial.println (" bit/s") ;
    Serial.print ("Exact bit rate ? ") ;
    Serial.println (settings.exactBitRate () ? "yes" : "no") ;
    Serial.print ("Sample point: ") ;
    Serial.print (settings.samplePointFromBitStart ()) ;
    Serial.println ("%") ;
  }else{
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }

  canMsg.ext = false;
  canMsg.id = 0x81;
  canMsg.len = 8;

  //first canners
  canMsg.data[0] = byte0(2000); //equivalente a torque=0
  canMsg.data[1] = byte1(2000,0,0); //antislope en 0 y gear en N
  canMsg.data[2] = byte2(15000); //speed en 0
  canMsg.data[3] = byte3(15000); //speed en 0
  canMsg.data[4] = byte4(1,0); //v2m en 1 y modo en ctrl de torque
  canMsg.data[5] = 0;
  canMsg.data[6] = byte6(canMsg.data[6]);
  byte checksum;
  checksum = canMsg.data[0] + canMsg.data[1] + canMsg.data[2] + canMsg.data[3] + canMsg.data[4] + canMsg.data[5] + canMsg.data[6];
  checksum = checksum ^ 0xFF;
  canMsg.data[7] = checksum;
}




void setupSensors(){
  for (int i=0;i<25;i++){
    memTl[i]=0;
  }
  for (int i=10;i<=11;i++){
    pinMode(i,INPUT);  //pull up row - Controller
  }
  pinMode(Rpin,OUTPUT); //relay shift R
  digitalWrite(Rpin, HIGH);
  pinMode(Fpin,OUTPUT); //relay shift F
  digitalWrite(Fpin, HIGH);
  pinMode(startRel,OUTPUT); //relay startstop
  digitalWrite(startRel, LOW);
}
//---------------------------------/settings--------------------------------//


//-----------------------------actuadores-----------------------------------//
void sender(){
  if (msgEna == 0){
    can.poll();
    const bool ok = can.tryToSend(canMsg);
    //Serial.println(ok);
    if (ok){
      lastSend = millis();
      printer81();
    }
  }
  
  if (shiftToll < 10){
    shiftToll++;
  }
}

void shifter(byte shiftNow, byte shiftWant){
  msgEna = 0;
  
  canMsg.data[0] = byte0(2015); //equivalente a torque=3
  canMsg.data[1] = byte1(2015,0,2); //antislope en 0 y gear en D
  canMsg.data[2] = byte2(15000); //speed en 0
  canMsg.data[3] = byte3(15000); //speed en 0
  canMsg.data[4] = byte4(1,2); //v2m en 1 y modo en ctrl de torque
  canMsg.data[5] = 0;
  canMsg.data[6] = byte6(canMsg.data[6]);
  byte checksum;
  checksum = canMsg.data[0] + canMsg.data[1] + canMsg.data[2] + canMsg.data[3] + canMsg.data[4] + canMsg.data[5] + canMsg.data[6];
  checksum = checksum ^ 0xFF;
  canMsg.data[7] = checksum;
  
  if (shiftToll < 10){
    return;
  }
  else {
    if      (    shiftNow == 2    ){  // i want D but...
      if (shiftWant == 1){            // ... im in R
        digitalWrite(Rpin, LOW);
        digitalWrite(Fpin, HIGH);
      } 
      else if (shiftWant == 0){       // ... im in N
        digitalWrite(Rpin, LOW);
        digitalWrite(Fpin, HIGH);
      }
    }
    else if (    shiftNow == 1    ){
      if (shiftWant == 2){    
        digitalWrite(Rpin, HIGH);
        digitalWrite(Fpin, LOW);
      } 
      else if (shiftWant == 0){
        digitalWrite(Rpin, HIGH);
        digitalWrite(Fpin, LOW);
      }
    }
    else if(    shiftNow == 0   ){
      if (shiftWant == 2){
        digitalWrite(Rpin, HIGH);
        digitalWrite(Fpin, LOW);
      }
      else if (shiftWant == 1){
        digitalWrite(Rpin, LOW);
        digitalWrite(Fpin, HIGH);
      }
    }
    else if(    shiftNow == 6   ){
      if (shiftWant == 2){
        digitalWrite(Rpin, HIGH);
        digitalWrite(Fpin, LOW);
      }
      else if (shiftWant == 1){
        digitalWrite(Rpin, LOW);
        digitalWrite(Fpin, HIGH);
      }
      else if (shiftWant == 0){
        digitalWrite(Rpin, HIGH);
        digitalWrite(Fpin, LOW);
      }
    }
    else if(    shiftNow == 7   ){
      if (shiftWant == 2){
        digitalWrite(Rpin, HIGH);
        digitalWrite(Fpin, LOW);
      }
      else if (shiftWant == 1){
        digitalWrite(Rpin, LOW);
        digitalWrite(Fpin, HIGH);
      }
      else if (shiftWant == 0){
        digitalWrite(Rpin, LOW);
        digitalWrite(Fpin, HIGH);
      }
    }
  }
  return;
}
//-----------------------------/actuadores-----------------------------------//



//----------------------------------data-calc---------------------------//
void hardread(){
  lastRead = millis();
  float phy_torq = phyT_calc(); //modif max
  //Serial.println(phy_torq);

  bool anslo = 0;
  byte gear = readGear();

  bool v2m = 1;
  byte mmode;

  if((gear == 2) || (gear == 1)){ 
    mmode=2;
  }
  else mmode=1;

  
  byte checksum;
  
  

  int can_torq = (phy_torq + 400) * 5;
  int can_speed = (phy_rpm + 15000);
  msgEna=0;
  canMsg.data[0] = byte0(can_torq);
  canMsg.data[1] = byte1(can_torq,anslo,2);
  canMsg.data[2] = byte2(can_speed);
  canMsg.data[3] = byte3(can_speed);
  canMsg.data[4] = byte4(v2m,mmode);
  canMsg.data[5] = 0;
  canMsg.data[6] = byte6(canMsg.data[6]);

  checksum = canMsg.data[0] + canMsg.data[1] + canMsg.data[2] + canMsg.data[3] + canMsg.data[4] + canMsg.data[5] + canMsg.data[6];
  checksum = checksum ^ 0xFF;
  
  canMsg.data[7] = checksum;
}


/////

float phyT_calc(){
  for (int i=0;i<24;i++){
    memTl[i] = memTl[i+1];
  }
  memTl[24] = analogRead(potT);
  int medTl = 0;
  for (int i=0;i<25;i++){
    medTl = medTl + memTl[i];
  }
  medTl = medTl/25;
  float torq = 0;
  if (medTl < 320){
    torq=0;
  } else if (medTl >= 320){
    torq = (10.0/590.0) * (medTl - 320); //map(medTl,312,910,0,30); //------------------MAX TORQ
    /*
     Para el modificar la relacion entre amplitud de DTS y amplitud de torque
     requerido se usa la ecuacion "y = m*(x-a)" donde 
      - y: torque
      - m = y1/(x1 - x0)}
      - x: la salida del DTS.
      - a = x0: valor de "x" donde "y" es cero
      - x1: valor de "x" donde se pone el maximo de "y".
      - y1: maximo valor de torque fisico.
     */
  }
  return torq;
}
/////

byte byte0(int torq){
  return (torq >> 4);
}


byte byte1(int torq, bool anslo, byte gear){
  byte b_1 = torq;
  b_1 = (b_1 << 4);
  
  if (anslo){
    b_1 = (b_1 | 8);
  }

  b_1 = (b_1 | gear);

  return b_1;
}

byte byte2(int rpm){
  return (rpm >> 8);
}

byte byte3(int rpm){
  return rpm;
}

byte byte4(bool v2m, byte mmode){
  byte b_4 = mmode;

  if (v2m){
    b_4 = (b_4 | 16);
  }

  return b_4;
}

byte byte6(byte msgCount){
  msgCount++;
  return (15 & msgCount);
}
//---------------------------------/data-calc---------------------------//



//---------------------------------sensing------------------------------//

void StartStop() {
  msgEna=0;
  if (!digitalRead(startBut) & !StartStopPress){
    StartStopPress = 1;
    if (!onoff){
      onoff = 1;
    } else {
      onoff = 0;
    }
  } else if (digitalRead(startBut) & StartStopPress){
    StartStopPress = 0;
  }
  if (onoff){
    digitalWrite(startRel,HIGH);
  }
  else {
    digitalWrite(startRel,LOW);
  }
}

byte readGear(){  //------------------revise: shift signal should first be stable?
  for (int i=0;i<9;i++){
    memSH[i] = memSH[i+1];
  }
  memSH[9] = analogRead(potS);
  int medSH = 0;
  for (int i=0;i<10;i++){
    medSH = medSH + memSH[i];
  }
  medSH = medSH/10;
  if (medSH > 600){
    return 2;
  }
  if (medSH < 440){
    return 1;
  }
  return 0;
  /*
  Serial.print(medSH);
  Serial.print(" ");
  Serial.println(analogRead(potS));
  
  
  /*for (int i=0;i<6;i++){
    if (digitalRead(i+16) == LOW){
      return i;
    }
  }//*/
  return 0;
}

byte readShifter(){
  int shiftLoc = analogRead(shiftPin);
  
  if (shiftLoc > 915){
    return 2;
  }
  else if (shiftLoc < 190){ 
    return 1;
  }
  else if ((shiftLoc > 400) && (shiftLoc <600)){
    return 0;
  }
  else if (shiftLoc <= 400){
    return 6;
  }
  else if (shiftLoc >= 600){
    return 7;
  }
  return 0;
}
//--------------------------------/sensing------------------------------//


//---------------------------------printing-----------------------------//

void printer81(){
  Serial.print("ID: ");
  Serial.print(canMsg.id, HEX); // print ID

  int hex_torq = canMsg.data[0] << 4;
  hex_torq = hex_torq | (canMsg.data[1] >> 4);
  float phy_torq = (hex_torq*0.2)-400;
  Serial.print("  Torque: ");
  Serial.print(phy_torq);

  int64_t hex_rpm = canMsg.data[2] << 8;
  hex_rpm = hex_rpm | canMsg.data[3];
  int phy_rpm = hex_rpm - 15000;
  Serial.print("  Speed: ");
  Serial.print(phy_rpm);

  char gear = 7 & canMsg.data[1];
  Serial.print("  Gear: ");
  Serial.print(gears[gear]);

  Serial.print("  Anti-Slip: ");
  if ((canMsg.data[1] & 0x08) == 0x08){
    Serial.print("ON");
  }
  else {
    Serial.print("OFF");
  }

  Serial.print("  VCU to MCU: ");
  if ((canMsg.data[4] & 0x10) == 0x10){
    Serial.print("Allow");
  }
  else {
    Serial.print("forbid");
  }
  
  byte mmode = canMsg.data[4] & 0xF;
  Serial.print("  Mode: ");
  Serial.print(modes[mmode]);


  checksum();

  Serial.println();
}


void checksum(){
  if ((canMsg.data[0] + canMsg.data[1] + canMsg.data[2] + canMsg.data[3] + canMsg.data[4] + canMsg.data[5] + canMsg.data[6]) ^ 0xFF == canMsg.data[7]){
    Serial.print("  CheckSum OK");
  }
  else {
    Serial.print("  CheckSum NOT OK");
  }
}

//---------------------------------printing-----------------------------//
