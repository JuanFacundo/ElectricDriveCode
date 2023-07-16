#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif

//—————LIBS——————————————————————————————————————————————————————————————
#include <ACAN2515.h>
#include <SimpleTimer.h>


//--------pines mcp--------//
// static const byte MCP2515_INT  = 1; // INT output of MCP2515  //MCP2515 CON INT
static const byte MCP2515_INT = 255; // INT output of MCP2515  //MCP2515 SIN INT
static const byte MCP2515_SCK = 2;  // SCK input of MCP2515
static const byte MCP2515_MOSI = 3; // SDI input of MCP2515
static const byte MCP2515_MISO = 4; // SDO output of MCP2515
static const byte MCP2515_CS = 5;   // CS input of MCP2515
#define pot0 26

//--------variables CAN------//
ACAN2515 can(MCP2515_CS, SPI, MCP2515_INT);
CANMessage canMsg;
static const uint32_t QUARTZ_FREQUENCY = 8UL * 1000UL * 1000UL; // 8 MHz //  MCP2515 Quartz: adaptado al diseño

//--------variables programa--------//
char gears[] = {'N','R','D','P','S','f','X'};
//char *modes[] = {"Initialization", "Standby", "TorqueCtrl", "SpeedCtrl", "Discharge", "Reserved", "!EMERGENCY SHUTDOWN!", "Allow12VBattPowerOff", "SystemFault", "SelfCheckField"};
//char *states[] = {"pow consum","pow gen","shutdown","READY"};
int phy_rpm=0;
byte count=0;
long toll = 1;
SimpleTimer sendit;
SimpleTimer readit;


//-------------SETUP----------//
void setup() {
  Serial.begin(38400);
  delay(1000);
  setupCAN();
  delay(300);
  setupSensors();
  setupID();
}




void loop() {  //-----------------------LOOP
  readit.run();
  sendit.run();
}







//---------------------------------------------------------------------------FUNCTIONS


void sender(){
  if (toll == 0){
    const bool ok = can.tryToSend(canMsg);
    //printer81();
    can.poll();
  }
}

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
  // const uint16_t errorCode = can.begin(settings, [] {can.isr();}); //MCP2515 CON INT
  can.begin(settings, NULL); // MCP2515 SIN INT
  
  sendit.setInterval(10, sender);   //-------------------ciclo de envio
}

void setupSensors(){
  for (int i=16;i<=21;i++){
    pinMode(i,INPUT);  //gear sensors
  }
  pinMode(6,INPUT);  //push button aka allower

  readit.setInterval(20, hardread); //-------------------ciclo para levantar datos fisicos
}

void setupID(){
  canMsg.ext = false;
  canMsg.id = 0x81;
  canMsg.len = 8;
}

//---------data-calc---------------//

void hardread(){
  float phy_torq = map(analogRead(pot0),4,1023,-51,10); //modif max

  bool anslo = 0;
  byte gear = readGear();

  bool v2m = 1;
  byte mmode;
  if(digitalRead(6) == LOW){
    mmode=2;
  }
  else mmode=1;
  

  
  byte checksum;
  
  

  int can_torq = (phy_torq + 400) * 5;
  int can_speed = (phy_rpm + 15000);
  toll=0;
  canMsg.data[0] = byte0(can_torq);
  canMsg.data[1] = byte1(can_torq,anslo,gear);
  canMsg.data[2] = byte2(can_speed);
  canMsg.data[3] = byte3(can_speed);
  canMsg.data[4] = byte4(v2m,mmode);
  canMsg.data[5] = 0;
  canMsg.data[6] = byte6(canMsg.data[6]);

  checksum = canMsg.data[0] + canMsg.data[1] + canMsg.data[2] + canMsg.data[3] + canMsg.data[4] + canMsg.data[5] + canMsg.data[6];
  checksum = checksum ^ 0xFF;
  
  canMsg.data[7] = checksum;
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

byte byte6(byte count){
  count++;
  return (15 & count);
}

//----------sensing----//
byte readGear(){
  for (int i=0;i<6;i++){
    if (digitalRead(i+16) == LOW){
      return i;
    }
  }
  return 0;
}

//--------printing------//
/*void printer81(){
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
}*/



void checksum(){
  if ((canMsg.data[0] + canMsg.data[1] + canMsg.data[2] + canMsg.data[3] + canMsg.data[4] + canMsg.data[5] + canMsg.data[6]) ^ 0xFF == canMsg.data[7]){
    Serial.print("  CheckSum OK");
  }
  else {
    Serial.print("  CheckSum NOT OK");
  }
}
