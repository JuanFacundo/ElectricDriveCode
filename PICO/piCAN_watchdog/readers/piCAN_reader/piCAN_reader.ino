#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif

//—————LIBS——————————————————————————————————————————————————————————————
#include <ACAN2515.h>

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

//--------variables-del-programa-----//
char gears[] = {'N','R','D','P','S','f','X'};
const char *modes[] = {"Initialization", "Standby", "TorqueCtrl", "SpeedCtrl", "Discharge", "Reserved", "!EMERGENCY SHUTDOWN!", "Allow12VBattPowerOff", "SystemFault", "SelfCheckField"};
const char *states[] = {"pow consum","pow gen","shutdown","READY"};


//-------------SETUP----------//
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  for (int i = 0; i < 10; i++)
  {
    delay(25);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.begin(38400);
  setupCAN();
}





void loop() {
  can.poll();
  if (can.available()) {
    can.receive(canMsg);
    //*
    if (canMsg.id == 0x081){
      printer81();
    } //*/
    //*
    else if (canMsg.id == 0x055){
      printer55();
    } //*/
    //*
    else if (canMsg.id == 0x097){
      printer97();
    } //*/
    //*
    else if (canMsg.id == 0x106){
      printer106();
    } //*/
    //*
    else if (canMsg.id == 0x655){
      printer655();
    } //*/
    //*
    else if (canMsg.id == 0x296){
      printer296();
    } //*/
    //*
    else if (canMsg.id == 0x297){
      printer297();
    } //*/
    //*
    else if (canMsg.id == 0x2B6){
      printer2B6();
    } //*/
    //*
    else if (canMsg.id == 0x2B7){
      printer2B7();
    } //*/
    //*
    else if (canMsg.id == 0x397){
      printer397();
    } //*/
    //*
    else if (canMsg.id == 0x398){
      printer398();
    } //*/
    /*
    else if (canMsg.id == 0x18EB2440){
      printer2240();
    }
    //*
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
     
  }
  
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
  ACAN2515Settings settings(QUARTZ_FREQUENCY, 250UL * 1000UL); // CAN bit rate 500 kb/s
  settings.mRequestedMode = ACAN2515Settings::NormalMode;      // NormalMode (default value), ListenOnlyMode,LoopBackMode, SleepMode.
  // const uint16_t errorCode = can.begin(settings, [] {can.isr();}); //MCP2515 CON INT
  can.begin(settings, NULL); // MCP2515 SIN INT
}
//-------------settings-----------//



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
  
  byte mode = canMsg.data[4] & 0xF;
  Serial.print("  Mode: ");
  Serial.print(modes[mode]);


  checksum();

  Serial.println();
}


////////




void printer55(){
  Serial.print("ID: ");
  Serial.print(canMsg.id, HEX); // print ID 

  int64_t hex_rpm = canMsg.data[0] << 8;
  hex_rpm = hex_rpm | canMsg.data[1];
  int phy_rpm = hex_rpm - 15000;
  Serial.print("  Speed: ");
  Serial.print(phy_rpm);

  int hex_torq = canMsg.data[2] << 4;
  hex_torq = hex_torq | (canMsg.data[3] >> 4);
  float phy_torq = (hex_torq*0.2)-400;
  Serial.print("  Torque: ");
  Serial.print(phy_torq);

  byte mode = canMsg.data[3] & 0xF;
  Serial.print("  Mode: ");
  Serial.print(modes[mode]);

  byte state = (canMsg.data[4] & 0x60) >> 5;
  Serial.print("  State: ");
  Serial.print(states[state]);

  byte F_rank = (canMsg.data[4] & 0x1C) >> 2;
  Serial.print("  Fault Rank: ");
  Serial.print(F_rank);

  int volt = ((canMsg.data[4] & 0x3) << 8) | canMsg.data[5];
  Serial.print("  Bus Voltage: ");
  Serial.print(volt);

  checksum();
  
  
  Serial.println();
}

void printer97 () {
  Serial.print("ID: ");
  Serial.print(canMsg.id, HEX); // print ID
  
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
}

void printer106(){
  Serial.print("ID: ");
  Serial.print(canMsg.id, HEX); // print ID
  
  byte FaultAmount = canMsg.data[0] >> 2;
  Serial.print("  cant: ");
  Serial.print(FaultAmount);
  if (FaultAmount != 0){
    byte FaultCode = ((canMsg.data[0] & 0x03) << 4) | (canMsg.data[1] >> 4);
    Serial.print("  F Code: 0x");
    Serial.print(FaultCode,HEX);
  }

  Serial.println();
}

void printer655(){
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
}


void printer296(){
  Serial.print("ID: ");
  Serial.print(canMsg.id,HEX); //print ID
  Serial.print("  ");
  
  byte DCDC_StsDCDCFault = canMsg.data[0] >> 6;
  const char *FS[] = {"Normal","Derating","Shutdown","Invalid"};
  Serial.print("DCDC Faults: ");
  Serial.print(FS[DCDC_StsDCDCFault]);
  Serial.print("  ");

  byte DCDC_StsCtrl = (canMsg.data[0] >> 4) & 0x03;
  const char *Ctrl[] = {"Inactive","Buck","Boost","Active Discharge"};
  Serial.print("DCDC State: ");
  Serial.print(Ctrl[DCDC_StsCtrl]);
  Serial.print("  ");

  int OxOutCurr = (canMsg.data[0] & 0x0F) << 8;
  OxOutCurr = OxOutCurr | canMsg.data[1];
  float DCDC_OutputCurrent = OxOutCurr*0.1;
  Serial.print("Out Curr: ");
  Serial.print(DCDC_OutputCurrent);
  Serial.print("  ");

  int OxInCurr = canMsg.data[2];
  float DCDC_InputCurrent = OxInCurr*0.1;
  Serial.print("In Curr: ");
  Serial.print(DCDC_InputCurrent);
  Serial.print("  ");

  int OxOutVolt = canMsg.data[3];
  float DCDC_OutputVolt = OxOutVolt*0.1;
  Serial.print("Out Volt: ");
  Serial.print(DCDC_OutputVolt);
  Serial.print("  ");

  int OxInVolt = canMsg.data[4] << 4;
  OxInVolt = OxInVolt | (canMsg.data[5] >> 4);
  float DCDC_InputVolt = OxInVolt*0.5;
  Serial.print("In Volt: ");
  Serial.print(DCDC_InputVolt);
  Serial.print("  ");

  int OxChgRel = (canMsg.data[5] & 0x0F) << 8;
  OxChgRel = OxChgRel | canMsg.data[6];
  float HVAS_BMS_DCChgRelayVolt = OxChgRel*0.5;
  Serial.print("Chg Rel: ");
  Serial.println(HVAS_BMS_DCChgRelayVolt);
}

void printer297(){
  Serial.print("ID: ");
  Serial.print(canMsg.id,HEX); //print ID
  Serial.print("  ");

  int DCDC_Temp = (canMsg.data[0] - 40);
  Serial.print("DCDC Temp: ");
  Serial.print(DCDC_Temp);
  Serial.print("°C  ");
  
  if ((canMsg.data[1] | canMsg.data[2] | canMsg.data[3]) == 0){
    Serial.println("no faults");
    return;
  }

  if((canMsg.data[0] & 0xC0) > 0){
    Serial.print("DCDC temp too high.  ");
  }
  if((canMsg.data[1] & 0x20) > 0){
    Serial.print("Imput volt too low alarm of DCDC.  ");
  }
  if((canMsg.data[1] & 0x10) > 0){
    Serial.print("Imput volt too high alarm of DCDC.  ");
  }
  if((canMsg.data[1] & 0x08) > 0){
    Serial.print("Input Cur. too high alarm of DCDC.  ");
  }
  if((canMsg.data[1] & 0x04) > 0){
    Serial.print("Output voltage too low alarm of DCDC.  ");
  }
  if((canMsg.data[1] & 0x02) > 0){
    Serial.print("Output volt too high alarm of DCDC.  ");
  }
  if((canMsg.data[1] & 0x01) > 0){
    Serial.print("Output Cur too high alarm of DCDC.  ");
  }
  if((canMsg.data[2] & 0x80) > 0){
    Serial.print("Output overpower output alarm of DCDC.  ");
  }
  if((canMsg.data[2] & 0x40) > 0){
    Serial.print("Hardware fault of DCDC.  ");
  }
  if((canMsg.data[2] & 0x20) > 0){
    Serial.print("Volt fault of battery.  ");
  }
  if((canMsg.data[2] & 0x10) > 0){
    Serial.print("12V supply fault.  ");
  }
  if((canMsg.data[2] & 0x08) > 0){
    Serial.print("Wakeup Input missing fault.  ");
  }
  if((canMsg.data[2] & 0x04) > 0){
    Serial.print("Communication missing fault.  ");
  }
  if((canMsg.data[2] & 0x02) > 0){
    Serial.print("Communication timeout fault.  ");
  }
  if((canMsg.data[2] & 0x01) > 0){
    Serial.print("Internal communication fault of DCDC.  ");
  }
  if((canMsg.data[3] & 0x80) > 0){
    Serial.print("Data storage fault of DCDC.  ");
  }
  if((canMsg.data[3] & 0x40) > 0){
    Serial.print("Output shortcircuit alarm of DCDC.  ");
  }
  if((canMsg.data[3] & 0x20) > 0){
    Serial.print("Output inversed alarm of DCDC.  ");
  }

  Serial.println();
}

void printer2B6(){
  Serial.print("ID: ");
  Serial.print(canMsg.id,HEX); //print ID
  Serial.print("  ");
  
  byte OBC_ChgFault = canMsg.data[0] >> 4;
  const char *CS[] = {"Init","Normal","Res","Derating","Res","Recov Pohib Chg","Prohib Chg","Res","Res","Res","Res","Res","Res","Res","Res","Invalid"};
  Serial.print("OBC Faults: ");
  Serial.print(CS[OBC_ChgFault]);
  Serial.print("  ");

  byte OBC_StsMode = canMsg.data[0] & 0x0F;
  const char *SM[] = {"Default","Init","Standby","Res","Res","SlowCarge","Res","Discharge","Res","Heating/Cooling","Res","Shutdown","AfterRun","End of Charge","PowerOff","Invlaid"};
  Serial.print("OBC Mode: ");
  Serial.print(SM[OBC_StsMode]);
  Serial.print("  ");

  byte OBC_StsCC = canMsg.data[1] >> 4;
  const char *CC[] = {"CC Open","Half Connect","Charging Allowed","Signal Fault","Discharge Allowed","Res","Res","Res","Res","Res","Res","Res","Res","Res","Res","Invalid"};
  Serial.print("CC state: ");
  Serial.print(CC[OBC_StsCC]);
  Serial.print("  ");

  byte OBC_StsCP = canMsg.data[1] & 0x0F;
  const char *CP[] = {"CP Open","S2 open","Charging Allowed","Signal Fault","Res","Res","Res","Res","Res","Res","Res","Res","Res","Res","Res","Invalid"};
  Serial.print("CP state: ");
  Serial.print(CP[OBC_StsCP]);
  Serial.print("  ");

  byte OxPWM = canMsg.data[2];
  float OBC_BMS_PWMCP = OxPWM * 0.4;
  Serial.print("CP PWM signal: ");
  Serial.print(OBC_BMS_PWMCP);
  Serial.print("%  ");

  byte OBC_StsChgLED = canMsg.data[3] >> 4;
  const char *LED[] = {"LIGHT OFF","White","Blue","Green flicker","Green","Red","Res","Res","Res","Res","Res","Res","Res","Res","Res","Invalid"};
  Serial.print("Charge LED status: ");
  Serial.print(LED[OBC_StsChgLED]);
  Serial.print("  ");

  byte OBC_VCU_StsElock = (canMsg.data[3] & 0x0C) >> 2;
  const char *ELOCK[] = {"Unlocked","Locked","Lock Fault","Invalid"};
  Serial.print("OBC Electronic Lock Status: ");
  Serial.print(ELOCK[OBC_VCU_StsElock]);
  Serial.print("  ");

  byte RollCount = canMsg.data[6] & 0x0F;
  Serial.print(RollCount);

  Serial.println();
}


void printer2B7(){
  Serial.print("ID: ");
  Serial.print(canMsg.id,HEX); //print ID
  Serial.print("  ");
  
  int aux = (canMsg.data[0] << 2) | (canMsg.data[1] >> 6);
  float OBC_MaxCurDC = aux*0.1;
  Serial.print("DC MAX out cur: ");
  Serial.print(OBC_MaxCurDC);
  Serial.print("A  ");

  aux = ((canMsg.data[1] & 0x3F) << 4) | (canMsg.data[2] >> 4);
  float OBC_MaxCurAC = aux*0.1;
  Serial.print("AC MAX out cur: ");
  Serial.print(OBC_MaxCurAC);
  Serial.print("A  ");

  aux = ((canMsg.data[2] & 0x0F) << 8) | canMsg.data[3];
  float OBC_ActVoltDC = aux*0.5;
  Serial.print("DC Volt: ");
  Serial.print(OBC_ActVoltDC);
  Serial.print("V  ");

  aux = (canMsg.data[4] << 2) | (canMsg.data[5] >> 6);
  float OBC_ActCurDC = aux*0.1;
  Serial.print("DC Cur: ");
  Serial.print(OBC_ActCurDC);
  Serial.print("A  ");

  aux = ((canMsg.data[5] & 0x3F) << 4) | (canMsg.data[6] >> 4);
  float OBC_ActVoltAC = aux*0.5;
  Serial.print("AC Volt: ");
  Serial.print(OBC_ActVoltAC);
  Serial.print("V  ");

  aux = ((canMsg.data[6] & 0x0F) << 6) | (canMsg.data[7] >> 2);
  float OBC_ActCurAC = aux*0.1;
  Serial.print("AC Cur: ");
  Serial.print(OBC_ActCurAC);
  Serial.println("A");
}

void printer397(){
  Serial.print("ID: ");
  Serial.print(canMsg.id,HEX); //print ID
  Serial.print("  ");
  
  int aux = canMsg.data[0];
  int OBC_PCBTemp1 = aux - 40;
  Serial.print("OBC PCB temp1: ");
  Serial.print(OBC_PCBTemp1);
  Serial.print("°C  ");
  
  aux = canMsg.data[1];
  int OBC_PCBTemp2 = aux - 40;
  Serial.print("OBC PCB temp2: ");
  Serial.print(OBC_PCBTemp2);
  Serial.print("°C  ");

  aux = canMsg.data[2];
  int OBC_InletTemp = aux - 40;
  Serial.print("OBC Inlet Temp: ");
  Serial.print(OBC_InletTemp);
  Serial.print("°C  ");

  aux = canMsg.data[3];
  int OBC_ACInletLTerminalTemp = aux - 40;
  Serial.print("AC L Termin. temp: ");
  Serial.print(OBC_ACInletLTerminalTemp);
  Serial.print("°C  ");

  aux = canMsg.data[4];
  int OBC_ACInletNTerminalTemp = aux - 40;
  Serial.print("AC N Termin. temp: ");
  Serial.print(OBC_ACInletNTerminalTemp);
  Serial.print("°C  ");

  aux = (canMsg.data[5] & 0x40) >> 6;
  byte OBC_InletTempSenFault = aux;
  Serial.print("Temp Sens fault?: ");
  const char *fault[] = {"Norm","ERROR"};
  Serial.print(fault[OBC_InletTempSenFault]);
  Serial.print("  ");

  byte OBC_LTerminalOverTemp = (canMsg.data[5] & 0x30) >> 4;
  Serial.print("L terminal temp too low?: ");
  const char *LTstate[] = {"Norm","OverTemp Level I","OverTemp Level II"};
  Serial.print(LTstate[OBC_LTerminalOverTemp]);
  Serial.print("  ");

  byte OBC_NTerminalOverTemp = (canMsg.data[5] & 0x30) >> 4;
  Serial.print("N terminal temp too low?: ");
  const char *NTstate[] = {"Norm","OverTemp Level I","OverTemp Level II"};
  Serial.println(NTstate[OBC_NTerminalOverTemp]);
}

void printer398(){
  Serial.print("ID: ");
  Serial.print(canMsg.id,HEX); //print ID
  Serial.print("  ");

  if ((canMsg.data[0] | canMsg.data[1] | canMsg.data[2]) == 0){
    Serial.println("no faults");
    return;
  }

  if((canMsg.data[0] & 0xC0) > 0){
    Serial.print("OBC temp too high.  ");
  }
  if((canMsg.data[0] & 0x20) > 0){
    Serial.print("S2 Control Fault.  ");
  }
  if((canMsg.data[0] & 0x10) > 0){
    Serial.print("12V Power supply Fault.  ");
  }
  if((canMsg.data[0] & 0x08) > 0){
    Serial.print("Input Volt too low.  ");    
  }
  if((canMsg.data[0] & 0x04) > 0){
    Serial.print("Imput Volt too high.  ");
  }
  if((canMsg.data[0] & 0x02) > 0){
    Serial.print("Imput Current too high.  ");
  }
  if((canMsg.data[0] & 0x01) > 0){
    Serial.print("AC input interrupt.  ");
  }
  if((canMsg.data[1] & 0x80) > 0){
    Serial.print("Output Volt too low.  ");
  }
  if((canMsg.data[1] & 0x40) > 0){
    Serial.print("Output Volt too high.  ");
  }
  if((canMsg.data[1] & 0x20) > 0){
    Serial.print("Output Curr too high.  ");
  }
  if((canMsg.data[1] & 0x10) > 0){
    Serial.print("Output shortcircuited.  ");
  }
  if((canMsg.data[1] & 0x08) > 0){
    Serial.print("Output inversed alarm of OBC.  ");
  }
  if((canMsg.data[1] & 0x04) > 0){
    Serial.print("Volt. fault of battery.  ");
  }
  if((canMsg.data[1] & 0x02) > 0){
    Serial.print("Wakeup input missing fault.  ");
  }
  if((canMsg.data[1] & 0x01) > 0){
    Serial.print("Communication timeout fault.  ");
  }
  if((canMsg.data[1] & 0x80) > 0){
    Serial.print("Communication missing fault.  ");
  }
  if((canMsg.data[0] & 0x40) > 0){
    Serial.print("Hardware fault of OBC.  ");
  }
  if((canMsg.data[0] & 0x20) > 0){
    Serial.print("Data storage fault of OBC.  ");
  }
  if((canMsg.data[0] & 0x10) > 0){
    Serial.print("Internal communication fault of OBC.  ");
  }

  Serial.println();
}

void printer2440(){
  Serial.print("ID: ");
  Serial.print(canMsg.id,HEX); //print ID
  Serial.print("  ");
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
