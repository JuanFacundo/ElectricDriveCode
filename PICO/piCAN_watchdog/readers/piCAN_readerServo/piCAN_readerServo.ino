#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif

//—————LIBS——————————————————————————————————————————————————————————————
#include <ACAN2515.h>
#include <Servo.h>
//#include <SimpleTimer.h>

//--------pines mcp--------//
// static const byte MCP2515_INT  = 1; // INT output of MCP2515  //MCP2515 CON INT
static const byte MCP2515_INT = 255; // INT output of MCP2515  //MCP2515 SIN INT
static const byte MCP2515_SCK = 2;  // SCK input of MCP2515
static const byte MCP2515_MOSI = 3; // SDI input of MCP2515
static const byte MCP2515_MISO = 4; // SDO output of MCP2515
static const byte MCP2515_CS = 5;   // CS input of MCP2515

//--------variables CAN------//
ACAN2515 can(MCP2515_CS, SPI, MCP2515_INT);
CANMessage canMsg;
static const uint32_t QUARTZ_FREQUENCY = 8UL * 1000UL * 1000UL; // 8 MHz //  MCP2515 Quartz: adaptado al diseño

//--------variables-del-Servo----------//
Servo myservo;
int reqAng = 0;

//--------variables-del-programa-----//
char gears[] = {'N','R','D','P','S','f','X'};
char *modes[] = {"Initialization", "Standby", "TorqueCtrl", "SpeedCtrl", "Discharge", "Reserved", "!EMERGENCY SHUTDOWN!", "Allow12VBattPowerOff", "SystemFault", "SelfCheckField"};
char *states[] = {"pow consum","pow gen","shutdown","READY"};
float phy_torq81 = 0;
float phy_torq55 = 0;
int phy_rpm=0;
long MCUlastTime = 0;
long VCUlastTime = 0;


//-------------SETUP----------//
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  for (int i = 0; i < 10; i++)
  {
    delay(25);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.begin(38400);
  setupCAN();
  myservo.attach(16);
  myservo.write(180);
}





void loop() {
  can.poll();
  if (can.available()) {
    can.receive(canMsg);
    //*
    if (canMsg.id == 0x081){
      printer81();
      VCUlastTime = millis();
      digitalWrite(LED_BUILTIN, HIGH);
    } //*/
    //*
    else if (canMsg.id == 0x055){
      printer55();
      MCUlastTime = millis();
    } //*/
    //*
    else if (canMsg.id == 0x097){
      printer97();
      MCUlastTime = millis();
    } //*/
    //*
    else if (canMsg.id == 0x106){
      printer106();
      MCUlastTime = millis();
    } //*/
    //*
    else if (canMsg.id == 0x655){
      printer655();
      MCUlastTime = millis();
    } //*/
    /*
    else {
      Serial.print("ID: ");
      Serial.print(canMsg.id, HEX); // print ID
    
    
      Serial.print(" DATA: ");
      for (int i = 0; i<canMsg.len; i++){
        if (canMsg.data[i]<16){
          Serial.print("0");
        }
        Serial.print(canMsg.data[i],HEX);
        Serial.print(" ");
      }

      Serial.println();     
    }
    //*/
    //Serial.print(phy_torq81);
    //Serial.print(" ");
    //Serial.println(phy_torq55);
  }
  /*
  if (millis() > (VCUlastTime + 150)){
    lcd.setCursor(19,2);
    lcd.print("0");
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    lcd.setCursor(19,2);
    lcd.print("1");
  }
  if (millis() > (MCUlastTime + 150)){
    lcd.setCursor(0,0);
    lcd.print("Motor off           ");
  }//*/

  int valu = map(phy_rpm,0,3100,0,90);
  if (valu>reqAng){
    reqAng++;
  } else if (valu<reqAng){
    reqAng-=1;
  }
  //Serial.print(valu);
  //Serial.print(" ");
  //Serial.println(reqAng);
  myservo.write(180-reqAng);
  
}


///////////////////////////////////////////////functions

//---------settings----------//
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
  // const uint16_t errorCode = can.begin(settings, [] {can.isr();}); //MCP2515 CON INT
  can.begin(settings, NULL); // MCP2515 SIN INT
}
//-------------settings-----------//



void printer81(){
  int hex_torq = canMsg.data[0] << 4;
  hex_torq = hex_torq | (canMsg.data[1] >> 4);
  phy_torq81 = (hex_torq*0.2)-400;

  int64_t hex_rpm = canMsg.data[2] << 8;
  hex_rpm = hex_rpm | canMsg.data[3];
  int phy_rpm = hex_rpm - 15000;

  char gear = 7 & canMsg.data[1];

  byte mode = canMsg.data[4] & 0xF;


  /*
  Serial.print("ID: ");
  Serial.print(canMsg.id, HEX); // print ID
  
  Serial.print("  Torque: ");
  Serial.print(phy_torq81);
  Serial.print("  Speed: ");
  Serial.print(phy_rpm);
  
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
  
  Serial.print("  Mode: ");
  Serial.print(modes[mode]);

  checksum();

  Serial.println();
  //*/
}


////////




void printer55(){
  int64_t hex_rpm = canMsg.data[0] << 8;
  hex_rpm = hex_rpm | canMsg.data[1];
  phy_rpm = hex_rpm - 15000;

  int hex_torq = canMsg.data[2] << 4;
  hex_torq = hex_torq | (canMsg.data[3] >> 4);
  phy_torq55 = (hex_torq*0.2)-400;

  byte mmode = canMsg.data[3] & 0xF;

  byte state = (canMsg.data[4] & 0x60) >> 5;

  byte F_rank = (canMsg.data[4] & 0x1C) >> 2;

  int volt = ((canMsg.data[4] & 0x3) << 8) | canMsg.data[5];

  
  /*
  Serial.print("ID: ");
  Serial.print(canMsg.id, HEX); // print ID 

  
  Serial.print("  Speed: ");*/
  Serial.println(phy_rpm);
  /*
  
  Serial.print("  Torque: ");
  Serial.print(phy_torq55);

  //*
  Serial.print("  Mode: ");
  Serial.print(modes[mmode]);


  Serial.print("  State: ");
  Serial.print(states[state]);


  Serial.print("  Fault Rank: ");
  Serial.print(F_rank);


  Serial.print("  Bus Voltage: ");
  Serial.print(volt);

  checksum();
  Serial.println();
  //*/
  

  
  
  digitalWrite(LED_BUILTIN, LOW);
  
}


void printer97 () {
  int phy_Mtorq = canMsg.data[0] << 4;
  phy_Mtorq = phy_Mtorq | (canMsg.data[1] >> 4);


  int phy_mtorq = (canMsg.data[1] & 0x0F) << 8;
  phy_mtorq = phy_mtorq | canMsg.data[2];
  phy_mtorq = phy_mtorq-400;


  int phy_curr = canMsg.data[3] << 4;
  phy_curr = phy_curr | (canMsg.data[4] >> 4);
  phy_curr = (phy_curr)-600;

  byte IGBT = (canMsg.data[4] & 0x0F) << 4;
  IGBT = IGBT | (canMsg.data[5] >> 4);
  IGBT = IGBT - 40;

  byte motemp = (canMsg.data[5] & 0x0F) << 4;
  motemp = motemp | (canMsg.data[6] >> 4);
  motemp = motemp - 40;



  /*
  Serial.print("ID: ");
  Serial.print(canMsg.id, HEX); // print ID
  
  
  Serial.print("  Max torque: ");
  Serial.print(phy_Mtorq);

  
  Serial.print("  min torque: ");
  Serial.print(phy_mtorq);

  
  Serial.print("  Bus Current: ");
  Serial.print(phy_curr);

  
  Serial.print("  IGBT temp: ");
  Serial.print(IGBT);
  
  
  Serial.print("  Motor temp: ");
  Serial.print(motemp);
  
  checksum();
  Serial.println();
  //*/
  
  digitalWrite(LED_BUILTIN, LOW);
}


void printer106(){
  byte FaultAmount = canMsg.data[0] >> 2;

  /*
  Serial.print("ID: ");
  Serial.print(canMsg.id, HEX); // print ID
  
  Serial.print("  cant: ");
  Serial.print(FaultAmount);
  if (FaultAmount != 0){
    byte FaultCode = ((canMsg.data[0] & 0x03) << 4) | (canMsg.data[1] >> 4);
    Serial.print("  F Code: 0x");
    Serial.print(FaultCode,HEX);
  }

  Serial.println();
  //*/

  
  digitalWrite(LED_BUILTIN, LOW);
}




void printer655(){
  /*
  Serial.print("ID: ");
  Serial.print(canMsg.id,HEX); //print ID

  Serial.print(" S.S.: ");
  Serial.print(canMsg.data[0],HEX);

  Serial.print("  Hardware v");
  byte u = canMsg.data[1] >> 4;
  byte dec = canMsg.data[1] & 15;
  Serial.print(u);
  Serial.print(".");
  Serial.print(dec);

  Serial.print("  Software v");
  u = canMsg.data[2] >> 4;
  dec = canMsg.data[2] & 15;
  Serial.print(u);
  Serial.print(".");
  Serial.print(dec);

  Serial.println();
  //*/
}




///////////

void checksum(){
  if ((canMsg.data[0] + canMsg.data[1] + canMsg.data[2] + canMsg.data[3] + canMsg.data[4] + canMsg.data[5] + canMsg.data[6]) ^ 0xFF == canMsg.data[7]){
    Serial.print("  CheckSum OK");
  }
  else {
    Serial.print("  CheckSum NOT OK");
  }
}
