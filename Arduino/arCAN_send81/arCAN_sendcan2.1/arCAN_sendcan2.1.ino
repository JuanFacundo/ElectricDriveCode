#include <SPI.h>
#include <mcp2515.h>

#define pot0 A0
#define pot1 A1  

struct can_frame canMsg;
MCP2515 mcp2515(10);
char gears[] = {'N','R','D','P','S','f','X'};
char *modes[] = {"Initialization", "Standby", "TorqueCtrl", "SpeedCtrl", "Discharge", "Reserved", "!EMERGENCY SHUTDOWN!", "Allow12VBattPowerOff", "SystemFault", "SelfCheckField"};
char *states[] = {"pow consum","pow gen","shutdown","READY"};

void setup() {
  canMsg.can_id  = 0x081;  ////////////CAN ID
  canMsg.can_dlc = 8;
  
  while (!Serial);
  Serial.begin(115200);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

  for (int i=3;i<=9;i++){
    pinMode(i,INPUT);
  }
  byte count=0;
}






void loop() {
  float phy_torq = map(analogRead(pot0),0,1023,-10,10); //modif max
  bool anslo = 0;
  byte gear = readGear();
  int phy_rpm = map(analogRead(pot1),0,1023,0,51);  //modif max
  bool v2m = 1;
  byte mode=2;

  byte checksum;
  
  

  int can_torq = (phy_torq + 400) * 5;
  int can_speed = (phy_rpm + 15000);
  
  
  
  canMsg.data[0] = byte0(can_torq);
  canMsg.data[1] = byte1(can_torq,anslo,gear);
  canMsg.data[2] = byte2(can_speed);
  canMsg.data[3] = byte3(can_speed);
  canMsg.data[4] = byte4(v2m,mode);
  canMsg.data[5] = 0;
  canMsg.data[6] = byte6(canMsg.data[6]);

  checksum = canMsg.data[0] + canMsg.data[1] + canMsg.data[2] + canMsg.data[3] + canMsg.data[4] + canMsg.data[5] + canMsg.data[6];
  checksum = checksum ^ 0xFF;
  
  canMsg.data[7] = checksum;

  mcp2515.sendMessage(&canMsg);


//  printer81();
//  printContent();
  
  delay(20);
}















///////////////////////////////////////////////




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

byte byte4(bool v2m, byte mode){
  byte b_4 = mode;

  if (v2m){
    b_4 = (b_4 | 16);
  }

  return b_4;
}

byte byte6(byte count){
  count++;
  return (15 & count);
}

///////////
byte readGear(){
  for (int i=0;i<6;i++){
    if (digitalRead(8-i) == LOW){
      return i;
    }
  }
  return 0;
}



//////////////////


void printer81(){
  Serial.print("ID: ");
  Serial.print(canMsg.can_id, HEX); // print ID

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
  
  byte mode = canMsg.data[4] & 0xF;
  Serial.print("  Mode: ");
  Serial.print(modes[mode]);


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




void printContent(){
  Serial.print("ID: ");
  Serial.print(canMsg.can_id, HEX); // print ID
    
    
  Serial.print(" DATA: ");
  for (int i = 0; i<canMsg.can_dlc; i++){
    if (canMsg.data[i]<16){
      Serial.print("0");
    }
    Serial.print(canMsg.data[i],HEX);
    Serial.print(" ");
  }
  Serial.println();
}
