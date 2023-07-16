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
CANMessage canMsg81, canMsg55, canMsg97;
static const uint32_t QUARTZ_FREQUENCY = 8UL * 1000UL * 1000UL; // 8 MHz //  MCP2515 Quartz: adaptado al diseño

//--------variables programa--------//
char gears[] = {'N','R','D','P','S','f','X'};
//char *modes[] = {"Initialization", "Standby", "TorqueCtrl", "SpeedCtrl", "Discharge", "Reserved", "!EMERGENCY SHUTDOWN!", "Allow12VBattPowerOff", "SystemFault", "SelfCheckField"};
//char *states[] = {"pow consum","pow gen","shutdown","READY"};
float phy_torq55=0;
float phy_torq=0;
float ask_torq=0; 
int phy_curr=0;
bool Iflag=0;
byte count=0;
long toll = 1;
SimpleTimer sendit;
SimpleTimer getit;
SimpleTimer plotit;
long GOtime = 0;
//long n=0; //número de muestreo
bool GO = 0;
long lastTime = 0;


//-------------SETUP----------//
void setup() {
  Serial.begin(38400);
  delay(1000);
  setupCAN();
  delay(300);
  setupSensors();
  setupID();
  plotit.setInterval(10, plotter);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  for (int i = 0; i < 200; i++)
  {
    delay(100);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1);
}




void loop() {  //-----------------------LOOP-------------------------------------------
  if ((digitalRead(6) == LOW) && (GO == 0)){
    GO = 1;
    GOtime = millis();
    digitalWrite(LED_BUILTIN,LOW);
  }
  if (GO){
    reader55();
    plotit.run();
    if (millis() - 20000 >= GOtime){  //----------15s de lectura
      //reset
      GO = 0;
      digitalWrite(LED_BUILTIN,HIGH);
    }
  }
  getit.run();
  sendit.run();

}







//---------------------------------------FUNCTIONS------------------------------------


//-----------------settings----------------//
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

  getit.setInterval(10, definer); //-------------------ciclo para levantar datos fisicos
}

void setupID(){
  canMsg81.ext = false;
  canMsg81.id = 0x81;
  canMsg81.len = 8;
}



//----------------CANners----------------//
void sender(){
  if (toll == 0){
    const bool ok = can.tryToSend(canMsg81);
    can.poll();
  }
}

void reader55(){
  can.poll(); //test without later
  if(can.available()){
    can.receive(canMsg55);
    if (canMsg55.id == 0x055){
      int hex_torq55 = canMsg55.data[2] << 4;
      hex_torq55 = hex_torq55 | (canMsg55.data[3] >> 4);
      phy_torq55 = (hex_torq55*0.2)-400;
    }
  }
}

bool reader97(){
  can.poll(); //test without later
  if(can.available()){
    can.receive(canMsg97);
    if (canMsg97.id == 0x097){
      phy_curr = canMsg97.data[3] << 4;
      phy_curr = phy_curr | (canMsg97.data[4] >> 4);
      phy_curr = (phy_curr)-600;
      return 1;
    }
  }
  return 0;
}


//---------------data-calc---------------//

void definer(){
  if ((millis() - 2000) <= GOtime){  //----------------2s de previa
    phy_torq = 0;
  }
  else if ((millis() - 10000) <= GOtime){  //----------10s de escalon
    phy_torq = 10;
  }
  else phy_torq = 0;

  Tfilter(phy_torq);
  int phy_rpm=0;
  bool anslo = 0;
  byte gear = 2;

  bool v2m = 1;
  byte mmode=readMode();
    
  byte checksum;

  int can_torq = (ask_torq + 400) * 5;
  int can_speed = (phy_rpm + 15000);
  toll=0;
  canMsg81.data[0] = byte0(can_torq);
  canMsg81.data[1] = byte1(can_torq,anslo,gear);
  canMsg81.data[2] = byte2(can_speed);
  canMsg81.data[3] = byte3(can_speed);
  canMsg81.data[4] = byte4(v2m,mmode);
  canMsg81.data[5] = 0;
  canMsg81.data[6] = byte6(canMsg81.data[6]);

  checksum = canMsg81.data[0] + canMsg81.data[1] + canMsg81.data[2] + canMsg81.data[3] + canMsg81.data[4] + canMsg81.data[5] + canMsg81.data[6];
  checksum = checksum ^ 0xFF;
  
  canMsg81.data[7] = checksum;
}

void Tfilter(float phy_torq){
  bool change = reader97();
  if (Iflag == 0){
    if (phy_curr < 10){
      ask_torq = phy_torq;
    }
    if (phy_curr > 10){
      Iflag = 1;
      ask_torq = phy_torq - 0.2*(phy_curr - 10);
    }
  }
  else if (Iflag == 1){
    if (phy_torq < ask_torq){
      Iflag = 0;
      ask_torq = phy_torq;
    }
    else if (change == 1){
      //ask_torq = ask_torq - 0.2*(phy_curr - 10); //proportional
      
      //*
      if (phy_curr > 10){
        ask_torq = ask_torq - 0.2;
      }
      if (phy_curr < 10){
        ask_torq = ask_torq + 0.2;
      }
      //*/
    }
  }
}



//----------hex-calcs-------------//
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

//----------sensing---------//
byte readMode(){
  for (int i=0;i<6;i++){
    if (digitalRead(i+16) == LOW){
      return i;
    }
  }
  return 1;
}

//--------printing------//
void plotter(){
    Serial.print(millis() - GOtime);
    Serial.print(" ");
    Serial.print(phy_torq);
    Serial.print(" ");
    Serial.print(ask_torq);
    Serial.print(" ");
    Serial.print(phy_torq55);
    Serial.println();
//    delay(1);
}

/* <- disable printer81 and checksum
void printer81(){
  Serial.print("ID: ");
  Serial.print(canMsg81.id, HEX); // print ID

  int hex_torq = canMsg81.data[0] << 4;
  hex_torq = hex_torq | (canMsg81.data[1] >> 4);
  float phy_torq = (hex_torq*0.2)-400;
  Serial.print("  Torque: ");
  Serial.print(phy_torq);

  int64_t hex_rpm = canMsg81.data[2] << 8;
  hex_rpm = hex_rpm | canMsg81.data[3];
  int phy_rpm = hex_rpm - 15000;
  Serial.print("  Speed: ");
  Serial.print(phy_rpm);

  char gear = 7 & canMsg81.data[1];
  Serial.print("  Gear: ");
  Serial.print(gears[gear]);

  Serial.print("  Anti-Slip: ");
  if ((canMsg81.data[1] & 0x08) == 0x08){
    Serial.print("ON");
  }
  else {
    Serial.print("OFF");
  }

  Serial.print("  VCU to MCU: ");
  if ((canMsg81.data[4] & 0x10) == 0x10){
    Serial.print("Allow");
  }
  else {
    Serial.print("forbid");
  }
  
  byte mmode = canMsg81.data[4] & 0xF;
  Serial.print("  Mode: ");
  Serial.print(modes[mmode]);


  checksum();

  Serial.println();
}



void checksum(){
  if ((canMsg81.data[0] + canMsg81.data[1] + canMsg81.data[2] + canMsg81.data[3] + canMsg81.data[4] + canMsg81.data[5] + canMsg81.data[6]) ^ 0xFF == canMsg81.data[7]){
    Serial.print("  CheckSum OK");
  }
  else {
    Serial.print("  CheckSum NOT OK");
  }
} 

//*/
