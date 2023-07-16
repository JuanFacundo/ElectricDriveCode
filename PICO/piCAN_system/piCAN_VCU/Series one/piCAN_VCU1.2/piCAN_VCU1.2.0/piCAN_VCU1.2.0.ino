#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif


//-------libs-----------//
#include <ACAN2515.h>
//-------/libs----------//


/////////////////////////////////////////////////////////////////GLOBALS/////////////////////////////////////////////////////////////////
//-------pins----------//
// static const byte MCP2515_INT  = 1; // INT output of MCP2515  //MCP2515 CON INT
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
//------/pins-----------//


//-------can-variables--//
ACAN2515 can(MCP2515_CS, SPI, MCP2515_INT);
CANMessage canMsgOut, canMsgIn;
static const uint32_t QUARTZ_FREQUENCY = 8UL * 1000UL * 1000UL; // 8 MHz //  MCP2515 Quartz: adaptado al diseño
//------/can-variables--//


//-------physical-variables----//
//id 81
int ask_torq = 0;
bool anslo = 0;
byte gear = 0;
bool v2m = 1;
byte askmode;


//id 55
int phy_rpm = 0;
float phy_torq = 0;
byte mmode = 0;
byte state = 0;
byte F_rank = 0;
int mvolt = 0;

//id 97
int phy_MaxTorq = 0;
int phy_minTorq = 0;
int phy_curr = 0;
byte IGBTtemp = 0;
byte moTemp = 0;

//id 106
byte FaultAmount = 0;
byte FaultCode = 0;

//actuators
byte shiftNow = 0;
//------/physical-variables----//


//-------timers-------//
#define sendInterval 10
long lastSend = millis();
#define readInterval 11
long lastRead = millis();
//------/timers-------//

//-------counters-&-flags-------//
byte msgCount=0;
byte shiftToll = 1;
byte msgEna = 1;
bool onoff = 0;
bool StartStopPress = 0;
int pressing = 0;
//------/counters-&-flags-------//

//--------------RAM-------------//
float memTL[3] = {0,0,0};
float memGear[3] = {0,0,0};
float memSH[3] = {0,0,0};
//-------------/RAM-------------//

/////////////////////////////////////////////////////////////////~GLOBALS/////////////////////////////////////////////////////////////////







/////////////////////////////////////////////////////////////////MAINS/////////////////////////////////////////////////////////////////
void setup(){
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(38400);

  /*
  while(!Serial){
    delay(1);
  }//*/
  
  setupCAN();
  setupSensors();
  delay(500);

  setupActuators();
  for (int i=0;i<10;i++){
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(100);
  }
}





void loop(){
  can.poll();
  CANreader();

  if(!onoff){
    StartStop();
    delay(10);
  }
  if (onoff){
    if (millis() >= (lastRead + readInterval)){
      byte shiftWant = readGear();
      
      if (shiftNow != shiftWant){
        shifter(shiftWant);
        shiftToll = 0;
        //Serial.println("gear change");
      }
      else{
        digitalWrite(Rpin,HIGH); //stop shift movement
        digitalWrite(Fpin,HIGH); //stop shift movement
        //Serial.println("drive");
        fetchPack();
        StartStop();
      }
    }

    if (StartStopPress){
      //smn
    }
    if (millis() >= (lastSend + sendInterval)){
      CANsender();
    }
  }
  
  //delay(1);
}



/////////////////////////////////////////////////////////////////~MAINS/////////////////////////////////////////////////////////////////













/////////////////////////////////////////////////////////////////FUNCTIONS/////////////////////////////////////////////////////////////////

//-------settings------//
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
  settings.mTransmitBuffer0Size = 30 ;
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

  //msgOUT ID
  canMsgOut.ext = false;
  canMsgOut.id = 0x81;
  canMsgOut.len = 8;
}

void setupSensors(){
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

void setupActuators(){
  do{//set the shift
    
    byte shiftWant = 0;//readGear();
    shiftNow = readShifter();
      
    if (shiftNow != shiftWant){
      shifter(shiftWant);
      shiftToll = 0;
      //Serial.println("gear change");
    } else {
      digitalWrite(Rpin,HIGH); //stop shift movement
      digitalWrite(Fpin,HIGH); //stop shift movement
      break;
    }
    
  }while(1);
  
}
//------/settings------//




//------canners-----//
void CANsender(){
  can.poll();
  if (msgEna == 0){
    const bool ok = can.tryToSend(canMsgOut);
    if (ok){
      lastSend = millis();
      digitalWrite(LED_BUILTIN, HIGH);
      //printer81();
    } else {
      digitalWrite(LED_BUILTIN, LOW);
      //Serial.println("tx full");
    }
  }
  
  if (shiftToll < 1){
    shiftToll++;
  }
}

void CANreader(){
  can.poll();
  if(can.available()){
    can.receive(canMsgIn);
    //*/
    if (canMsgIn.id == 0x055){
      receive55();
      //Serial.println(phy_rpm);
      //printer55();
    } //*/
    //*
    else if (canMsgIn.id == 0x097){
      receive97();
      //printer97();
    } //*/
    //*
    else if (canMsgIn.id == 0x106){
      receive106();
    } //*/
    /*
    else if (canMsgIn.id == 0x296){
      receive296();
    } //*/
    /*
    else {
      Serial.print("ID: ");
      Serial.print(canMsgIn.id, HEX); // print ID
    
    
      Serial.print(" DATA: ");
      for (int i = 0; i<canMsgIn.len; i++){
        if (canMsgIn.data[i]<16){
          Serial.print("0");
        }
        Serial.print(canMsgIn.data[i],HEX);
        Serial.print(" ");
      }

      Serial.println();
    } //*/
  }
}
//-----/canners-----//



//----------actuadores----------//
void shifter(byte shiftWant){
  msgEna = 0;
  
  canMsgOut.data[0] = byte0(2030); //equivalente a torque=3 -----------------------------REVISE----------------------!!!!!!!!!!!!
  canMsgOut.data[1] = byte1(2030,0,2); //antislope en 0 y gear en D
  canMsgOut.data[2] = byte2(15000); //speed en 0
  canMsgOut.data[3] = byte3(15000); //speed en 0
  canMsgOut.data[4] = byte4(1,2); //v2m en 1 y modo en ctrl de torque
  canMsgOut.data[5] = 0;
  canMsgOut.data[6] = byte6(canMsgOut.data[6]);
  byte checksum;
  checksum = canMsgOut.data[0] + canMsgOut.data[1] + canMsgOut.data[2] + canMsgOut.data[3] + canMsgOut.data[4] + canMsgOut.data[5] + canMsgOut.data[6];
  checksum = checksum ^ 0xFF;
  canMsgOut.data[7] = checksum;
  
  if (shiftToll < 1){
    return;
  }
  else {
    if      (    shiftNow == 2    ){  // i want F but...
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
    shiftNow = readShifter();
  }
  return;
}
//---------/actuadores----------//


//-----data-calc-----//
void fetchPack(){
  lastRead = millis();
  ask_torq = phyTorqRead();

  anslo = 0;
  gear = readGear();

  v2m = 1;

  if((gear == 2) || (gear == 1)){ 
    askmode=2;
  }
  else askmode=1;

  
  byte checksum;
  
  

  int can_torq = (ask_torq + 400) * 5;
  int can_speed = (0 + 15000);
  msgEna=0;
  canMsgOut.data[0] = byte0(can_torq);
  canMsgOut.data[1] = byte1(can_torq,anslo,2);
  canMsgOut.data[2] = byte2(can_speed);
  canMsgOut.data[3] = byte3(can_speed);
  canMsgOut.data[4] = byte4(v2m,askmode);
  canMsgOut.data[5] = 0;
  canMsgOut.data[6] = byte6(canMsgOut.data[6]);

  checksum = canMsgOut.data[0] + canMsgOut.data[1] + canMsgOut.data[2] + canMsgOut.data[3] + canMsgOut.data[4] + canMsgOut.data[5] + canMsgOut.data[6];
  checksum = checksum ^ 0xFF;
  
  canMsgOut.data[7] = checksum;
}




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



void receive55(){
  int64_t hex_rpm = canMsgIn.data[0] << 8;
  hex_rpm = hex_rpm | canMsgIn.data[1];
  phy_rpm = hex_rpm - 15000;

  int hex_torq = canMsgIn.data[2] << 4;
  hex_torq = hex_torq | (canMsgIn.data[3] >> 4);
  phy_torq = (hex_torq*0.2)-400;

  mmode = canMsgIn.data[3] & 0xF;

  state = (canMsgIn.data[4] & 0x60) >> 5;

  F_rank = (canMsgIn.data[4] & 0x1C) >> 2;

  mvolt = ((canMsgIn.data[4] & 0x3) << 8) | canMsgIn.data[5];
  
}

void receive97 () {
  phy_MaxTorq = canMsgIn.data[0] << 4;
  phy_MaxTorq = phy_MaxTorq | (canMsgIn.data[1] >> 4);

  phy_minTorq = (canMsgIn.data[1] & 0x0F) << 8;
  phy_minTorq = phy_minTorq | canMsgIn.data[2];
  phy_minTorq = phy_minTorq-400;

  phy_curr = canMsgIn.data[3] << 4;
  phy_curr = phy_curr | (canMsgIn.data[4] >> 4);
  phy_curr = (phy_curr)-600;

  IGBTtemp = (canMsgIn.data[4] & 0x0F) << 4;
  IGBTtemp = IGBTtemp | (canMsgIn.data[5] >> 4);
  IGBTtemp = IGBTtemp - 40;
  
  moTemp = (canMsgIn.data[5] & 0x0F) << 4;
  moTemp = moTemp | (canMsgIn.data[6] >> 4);
  moTemp = moTemp - 40;

  
}

void receive106(){
  FaultAmount = canMsgIn.data[0] >> 2;
  if (FaultAmount != 0){
    FaultCode = ((canMsgIn.data[0] & 0x03) << 4) | (canMsgIn.data[1] >> 4);
    Serial.print("  F Code: 0x");
    Serial.print(FaultCode,HEX);
    Serial.println();
  } else {
    FaultCode = 0;
  }
}


/*
void receive296(){
  byte DCDC_StsDCDCFault = canMsgIn.data[0] >> 6;
  const char *FS[] = {"Normal","Derating","Shutdown","Invalid"};
  Serial.print("DCDC Faults: ");
  Serial.print(FS[DCDC_StsDCDCFault]);
  Serial.print(" ");

  byte DCDC_StsCtrl = (canMsgIn.data[0] >> 4) & 0x03;
  const char *Ctrl[] = {"Inactive","Buck","Boost","Active Discharge"};
  Serial.print("DCDC State: ");
  Serial.print(Ctrl[DCDC_StsCtrl]);
  Serial.print(" ");

  int OxOutCurr = (canMsgIn.data[0] & 0x0F) << 8;
  OxOutCurr = OxOutCurr | canMsgIn.data[1];
  float DCDC_OutputCurrent = OxOutCurr*0.1;
  Serial.print("Out Curr: ");
  Serial.print(DCDC_OutputCurrent);
  Serial.print(" ");

  int OxInCurr = canMsgIn.data[2];
  float DCDC_InputCurrent = OxInCurr*0.1;
  Serial.print("In Curr: ");
  Serial.print(DCDC_InputCurrent);
  Serial.print(" ");

  int OxOutVolt = canMsgIn.data[3];
  float DCDC_OutputVolt = OxOutVolt*0.1;
  Serial.print("Out Volt: ");
  Serial.print(DCDC_OutputVolt);
  Serial.print(" ");

  int OxInVolt = canMsgIn.data[4] << 4;
  OxInVolt = OxInVolt | (canMsgIn.data[5] >> 4);
  float DCDC_InputVolt = OxInVolt*0.5;
  Serial.print("In Volt: ");
  Serial.print(DCDC_InputVolt);
  Serial.print(" ");

  int OxChgRel = (canMsgIn.data[5] & 0x0F) << 8;
  OxChgRel = OxChgRel | canMsgIn.data[6];
  float HVAS_BMS_DCChgRelayVolt = OxChgRel*0.5;
  Serial.print("Chg Rel: ");
  Serial.println(HVAS_BMS_DCChgRelayVolt);
}//*/
//----/data-calc-----//





//-----------------sensing---------------//

void StartStop() {
  if (!digitalRead(startBut) & !StartStopPress){
    StartStopPress = 1;
  } else if (digitalRead(startBut) & StartStopPress){
    StartStopPress = 0;
    pressing = 0;
  }
  
  if (StartStopPress){
    pressing++;

    if (pressing == 100){ 
      if (!onoff){
        onoff = 1;
      } else {
        onoff = 0;
      }
    }
  }
  
  if (onoff){
    digitalWrite(startRel,HIGH);
  }
  else {
    digitalWrite(startRel,LOW);
  }
}


int phyTorqRead(){
  memTL[2] = memTL[1];
  memTL[1] = memTL[0];
  memTL[0] = analogRead(potT) + 1.556914*memTL[1] - 0.62659*memTL[2];
  int medTl = round(0.01742*memTL[0] + 0.0348374*memTL[1] + 0.0174188*memTL[2]);
  int torq = 0;
  if (medTl < 320){
    torq=0;
  } else if (medTl >= 320){
    torq = round((50.0/590.0) * (medTl - 320)); //map(medTl,312,910,0,30); //------------------MAX TORQ
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
  //Serial.println(torq);
  
  return torq;
}

byte readGear(){
  memGear[2] = memGear[1];
  memGear[1] = memGear[0];
  memGear[0] = analogRead(potS) + 1.556914*memGear[1] - 0.62659*memGear[2];
  int medGear = round(0.01742*memGear[0] + 0.0348374*memGear[1] + 0.0174188*memGear[2]);
  
  if (medGear > 600){
    return 2;
  }
  if (medGear < 440){
    return 1;
  }
  return 0;
}

byte readShifter(){
  memSH[2] = memSH[1];
  memSH[1] = memSH[0];
  memSH[0] = analogRead(shiftPin) + 1.556914*memSH[1] - 0.62659*memSH[2];
  int shiftLoc = round(0.01742*memSH[0] + 0.0348374*memSH[1] + 0.0174188*memSH[2]);

  
  if (shiftLoc > 900){
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
//----------------/sensing---------------//


//-----------------printing--------------//
void printer81(){
  Serial.print("ID: 81  ");
  Serial.print("  Torque: ");
  Serial.print(ask_torq);
  Serial.print("  Speed: ");
  Serial.print(0);
  Serial.print("  Gear: ");
  char gears[] = {'N','R','D','P','S','f','X'};
  Serial.print(gears[gear]);
  Serial.print("  Anti-Slip: OFF");
  Serial.print("  VCU to MCU: Allow");
  Serial.print("  Mode: ");
  const char *modes[] = {"Initialization", "Standby", "TorqueCtrl", "SpeedCtrl", "Discharge", "Reserved", "!EMERGENCY SHUTDOWN!", "Allow12VBattPowerOff", "SystemFault", "SelfCheckField"};
  Serial.print(modes[askmode]);
  Serial.println();
}

void printer55(){
  Serial.print("ID: 55  Speed: ");
  Serial.print(phy_rpm);
  Serial.print("  Torque: ");
  Serial.print(phy_torq);
  Serial.print("  Mode: ");
  const char *modes[] = {"Initialization", "Standby", "TorqueCtrl", "SpeedCtrl", "Discharge", "Reserved", "!EMERGENCY SHUTDOWN!", "Allow12VBattPowerOff", "SystemFault", "SelfCheckField"};
  Serial.print(modes[mmode]);
  Serial.print("  State: ");
  const char *states[] = {"pow consum","pow gen","shutdown","READY"};
  Serial.print(states[state]);
  Serial.print("  Fault Rank: ");
  Serial.print(F_rank);
  Serial.print("  Bus Voltage: ");
  Serial.print(mvolt);
  Serial.println();
}

void printer97(){
  Serial.print("ID: 97  Max Torq: ");
  Serial.print(phy_MaxTorq);
  Serial.print("  min torque: ");
  Serial.print(phy_minTorq);
  Serial.print("  Bus Current: ");
  Serial.print(phy_curr);
  Serial.print("  IGBT temp: ");
  Serial.print(IGBTtemp);
  Serial.print("  Motor temp: ");
  Serial.print(moTemp);  
  Serial.println();
}
/////////////////////////////////////////////////////////////////~FUNCTIONS/////////////////////////////////////////////////////////////////
