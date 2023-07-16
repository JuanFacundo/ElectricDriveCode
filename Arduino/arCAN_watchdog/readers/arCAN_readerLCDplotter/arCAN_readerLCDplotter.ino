#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <mcp2515.h>

LiquidCrystal_I2C lcd(0x3F, 20, 4);

char gears[] = {'N','R','D','P','S','f','X'};
char *modes[] = {"Initialization", "Standby", "TorqueCtrl", "SpeedCtrl", "Discharge", "Reserved", "!EMERGENCY SHUTDOWN!", "Allow12VBattPowerOff", "SystemFault", "SelfCheckField"};
char *states[] = {"pow consum","pow gen","shutdown","READY"};
float phy_torq81=0;
int phy_rpm81=0;
int phy_rpm = 0;
float phy_torq=0;


struct can_frame canMsg;
MCP2515 mcp2515(10);





void setup() {
  
  Serial.begin(115200);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();


  lcd.begin();
  lcd.backlight();
  
}






void loop() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    if (canMsg.can_id == 0x081){
      printer81();
    }
    else if (canMsg.can_id == 0x055){
      printer55();
    }
//    else if (canMsg.can_id == 0x097){
//      printer97();
//    }
//    else if (canMsg.can_id == 0x106){
//      printer106();
//    }
//    else if (canMsg.can_id == 0x655){
//      printer655();
//    }
    else {
//      Serial.print("ID: ");
//      Serial.print(canMsg.can_id, HEX); // print ID
    
    
//      Serial.print(" DATA: ");
      for (int i = 0; i<canMsg.can_dlc; i++){
        if (canMsg.data[i]<16){
//          Serial.print("0");
        }
//        Serial.print(canMsg.data[i],HEX);
//        Serial.print(" ");
      }

      Serial.println();     
    }
    plotter();
  }
  
}




///////////////////////////////////////////////functions




void printer81(){
//  Serial.print("ID: ");
//  Serial.print(canMsg.can_id, HEX); // print ID

  int hex_torq = canMsg.data[0] << 4;
  hex_torq = hex_torq | (canMsg.data[1] >> 4);
  phy_torq81 = (hex_torq*0.2)-400;
//  Serial.print("  Torque: ");
//  Serial.print(phy_torq81);

  int64_t hex_rpm = canMsg.data[2] << 8;
  hex_rpm = hex_rpm | canMsg.data[3];
  phy_rpm81 = hex_rpm - 15000;
//  Serial.print("  Speed: ");
//  Serial.print(" ");
//  Serial.print(phy_rpm81);
//  Serial.print(" ");

  char gear = 7 & canMsg.data[1];
//  Serial.print("  Gear: ");
//  Serial.print(gears[gear]);

//  Serial.print("  Anti-Slip: ");
  if ((canMsg.data[1] & 0x08) == 0x08){
//    Serial.print("ON");
  }
  else {
//    Serial.print("OFF");
  }

//  Serial.print("  VCU to MCU: ");
  if ((canMsg.data[4] & 0x10) == 0x10){
//    Serial.print("Allow");
  }
  else {
//    Serial.print("forbid");
  }
  
  byte mode = canMsg.data[4] & 0xF;
//  Serial.print("  Mode: ");
//  Serial.print(modes[mode]);


//  checksum();

//  Serial.println();
}


////////




void printer55(){
//  Serial.print("ID: ");
//  Serial.print(canMsg.can_id, HEX); // print ID 

  int64_t hex_rpm = canMsg.data[0] << 8;
  hex_rpm = hex_rpm | canMsg.data[1];
  phy_rpm = hex_rpm - 15000;
//  Serial.print("  Speed: ");
//  Serial.print(phy_rpm);
//  Serial.print(" ");

  int hex_torq = canMsg.data[2] << 4;
  hex_torq = hex_torq | (canMsg.data[3] >> 4);
  phy_torq = (hex_torq*0.2)-400;
//  Serial.print("  Torque: ");
//  Serial.print(phy_torq);
//  Serial.print(" ");

  byte mode = canMsg.data[3] & 0xF;
//  Serial.print("  Mode: ");
//  Serial.print(modes[mode]);

  byte state = (canMsg.data[4] & 0x60) >> 5;
//  Serial.print("  State: ");
//  Serial.print(states[state]);

  byte F_rank = (canMsg.data[4] & 0x1C) >> 2;
//  Serial.print("  Fault Rank: ");
//  Serial.print(F_rank);

  int volt = ((canMsg.data[4] & 0x3) << 8) | canMsg.data[5];
//  Serial.print("  Bus Voltage: ");
//  Serial.print(volt);

//  checksum();
  
  lcdPrinter55(phy_rpm, phy_torq, mode,state,volt);
  
//  Serial.println();
}

void lcdPrinter55(int rpm, float torq, byte mode, byte state, int volt){
  lcd.setCursor(0,0);
  lcd.print("rpm:");
  char c[6];
  dtostrf(rpm,6,0,c);
  lcd.print(c);
  lcd.setCursor(10,0);
  lcd.print("| t:");
  dtostrf(torq,6,1,c);
  lcd.print(c);

  lcd.setCursor(0,1);
  lcd.print("M:");
  dtostrf(mode,1,0,&c[0]);
  lcd.print(&c[0]);
  lcd.print(" St:");
  dtostrf(state,1,0,&c[0]);
  lcd.print(&c[0]);
  lcd.print(" busV:");
  dtostrf(volt,4,0,c);
  lcd.print(c);
}

void printer97() {
  Serial.print("ID: ");
  Serial.print(canMsg.can_id, HEX); // print ID
  
  int phy_Mtorq = canMsg.data[0] << 4;
  phy_Mtorq = phy_Mtorq | (canMsg.data[1] >> 4);
  Serial.print("  Max torque: ");
  Serial.print(phy_Mtorq);

  int phy_mtorq = (canMsg.data[1] & 0x0F) << 8;
  phy_mtorq = phy_mtorq | canMsg.data[2];
  phy_mtorq = phy_mtorq-400;
  Serial.print("  min torque: ");
  Serial.print(phy_mtorq);

  int phy_curr = canMsg.data[3] << 4;
  phy_curr = phy_curr | (canMsg.data[4] >> 4);
  phy_curr = (phy_curr)-600;
  Serial.print("  Bus Current: ");
  Serial.print(phy_curr);

  byte IGBT = (canMsg.data[4] & 0x0F) << 4;
  IGBT = IGBT | (canMsg.data[5] >> 4);
  IGBT = IGBT - 40;
  Serial.print("  IGBT temp: ");
  Serial.print(IGBT);
  
  byte motemp = (canMsg.data[5] & 0x0F) << 4;
  motemp = motemp | (canMsg.data[6] >> 4);
  motemp = motemp - 40;
  Serial.print("  Motor temp: ");
  Serial.print(motemp);
  
  checksum();

  Serial.println();

  lcdPrinter97(phy_curr, motemp, IGBT);
}

void lcdPrinter97(int I, byte motemp, byte IGBT){
  lcd.setCursor(0,2);
  lcd.print("I:");
  char c[4];
  dtostrf(I,4,0,c);
  lcd.print(c);
  lcd.setCursor(0,3);
  lcd.print("mTemp:");
  dtostrf(motemp,3,0,c);
  lcd.print(c);
  lcd.print("C");
  lcd.print(" cTmp:");
  dtostrf(IGBT,3,0,c);
  lcd.print(c);
  lcd.print("C");
}



void plotter(){
  Serial.print(phy_torq81);
  Serial.print(" ");
  Serial.print(0);
  Serial.print(" ");
  Serial.print(phy_torq);
  Serial.println();
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
