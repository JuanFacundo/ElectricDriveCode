#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <mcp2515.h>
#include <SimpleTimer.h>

LiquidCrystal_I2C lcd(0x3F, 20, 4);

char gears[] = {'N','R','D','P','S','f','X'};
char *modes[] = {"Initialization", "Standby", "TorqueCtrl", "SpeedCtrl", "Discharge", "Reserved", "!EMERGENCY SHUTDOWN!", "Allow12VBattPowerOff", "SystemFault", "SelfCheckField"};
char *states[] = {"pow consum","pow gen","shutdown","READY"};
float phy_torq81=0;
int phy_rpm81=0;
int phy_rpm = 0;
float phy_torq=0;
bool pressFlag = 0;
long GOtime = 0;


struct can_frame canMsg;
MCP2515 mcp2515(10);




//----------setup--------//
void setup() {
  pinMode(6, INPUT);
  Serial.begin(115200);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

  
  lcd.begin();
  lcd.backlight();
}






void loop() {
  if (digitalRead(6) == LOW){
    if (!pressFlag){
      pressFlag = !pressFlag;
      GOtime = millis();
    }
    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
      if (canMsg.can_id == 0x081){
        printer81();
      }
      else if (canMsg.can_id == 0x055){
       printer55();
      }
      else if (canMsg.can_id == 0x097){
        printer97();
      }
      else if (canMsg.can_id == 0x106){
        printer106();
      }
      plotter();
    }
  }
  else if (digitalRead(6) == HIGH){
    if (pressFlag){
      pressFlag = !pressFlag;
    }
  }
  
  
}








//---------------------------------------------------functions-------------------------//

//---------------printers--------------//

void plotter(){
  Serial.print(millis()-GOtime);
  //Serial.print(0);
  Serial.print(" ");
  Serial.print(phy_torq81);
  Serial.print(" ");
  Serial.print(phy_torq);
  Serial.println();
}

void printer81(){

  int hex_torq = canMsg.data[0] << 4;
  hex_torq = hex_torq | (canMsg.data[1] >> 4);
  phy_torq81 = (hex_torq*0.2)-400;

  int64_t hex_rpm = canMsg.data[2] << 8;
  hex_rpm = hex_rpm | canMsg.data[3];
  phy_rpm81 = hex_rpm - 15000;

  char gear = 7 & canMsg.data[1];

  byte mode = canMsg.data[4] & 0xF;

}

void printer55(){


  int64_t hex_rpm = canMsg.data[0] << 8;
  hex_rpm = hex_rpm | canMsg.data[1];
  phy_rpm = hex_rpm - 15000;

  int hex_torq = canMsg.data[2] << 4;
  hex_torq = hex_torq | (canMsg.data[3] >> 4);
  phy_torq = (hex_torq*0.2)-400;

  byte mode = canMsg.data[3] & 0xF;
  
  byte state = (canMsg.data[4] & 0x60) >> 5;

  byte F_rank = (canMsg.data[4] & 0x1C) >> 2;

  int volt = ((canMsg.data[4] & 0x3) << 8) | canMsg.data[5];
  
  lcdPrinter55(phy_rpm, phy_torq, mode,state,volt);
  
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
  
  lcd.setCursor(6,2);
  lcd.print(" busV:");
  dtostrf(volt,4,0,c);
  lcd.print(c);
}

void printer97() {
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

void printer106(){
  byte FaultAmount = canMsg.data[0] >> 2;
  lcdPrinter106(FaultAmount);
}

void lcdPrinter106(byte FA){
  lcd.setCursor(8,1);
  lcd.print(" Faults:");
  char c[2];
  dtostrf(FA,2,0,c);
  lcd.print(c);
}
