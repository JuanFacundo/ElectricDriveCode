#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsg81;
MCP2515 mcp2515(10);


void setup() {
  canMsg81.can_id  = 0x081;
  canMsg81.can_dlc = 8;
  
  while (!Serial);
  Serial.begin(115200);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  
  byte count=0;
}






void loop() {
  
  float phy_torq = 10.0;
  bool anslo = 1;
  byte gear = 4;
  int phy_rpm = 30;
  bool v2m = 1;
  byte mode=0;

  byte checksum;
  
  

  int can_torq = (phy_torq + 400) * 5;
  int can_speed = (phy_rpm + 15000);
  
  
  
  canMsg81.data[0] = byte0(can_torq);
  canMsg81.data[1] = byte1(can_torq,anslo,gear);
  canMsg81.data[2] = byte2(can_speed);
  canMsg81.data[3] = byte3(can_speed);
  canMsg81.data[4] = byte4(v2m,mode);
  canMsg81.data[5] = 0;
  canMsg81.data[6] = byte6(canMsg81.data[6]);

  checksum = canMsg81.data[0] + canMsg81.data[1] + canMsg81.data[2] + canMsg81.data[3] + canMsg81.data[4] + canMsg81.data[5] + canMsg81.data[6];
  checksum = checksum ^ 0xFF;
  
  canMsg81.data[7] = checksum;

  mcp2515.sendMessage(&canMsg81);
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
