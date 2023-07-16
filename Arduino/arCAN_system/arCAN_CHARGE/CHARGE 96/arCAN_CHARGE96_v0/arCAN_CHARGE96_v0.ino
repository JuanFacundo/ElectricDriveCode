//CHARGER 96
#include <SPI.h>
#include <mcp2515.h>

//------------phy-variables-----------//
int n_celdas = 32;
float vCelda_max = 3.65;
float vCelda_min = 2.8;
float I_max = 10.0;  //presicion 0.1

float vBat_min = vCelda_min * n_celdas; //precision 0.1
float vBat_max = vCelda_max * n_celdas; //precision 0.1

int rampaZone = 15; //faltando _ porciento de carga se entra en la rampa.

float phyBatV = 0; //precision de 0.1
//-----------!phy-variables-----------//

//------------can-variables-----------//
struct can_frame canMsgIN;
struct can_frame canMsgOUT;
MCP2515 mcp2515(10);                            /////////////FALTA EL BAUDRATE
//-----------!can-variables-----------//

//------------timers-and-flags--------//
long last_send = millis();
bool GO = 1;
//-----------!timers-and-flags--------//

void setup() {
  Serial.begin(38400);

  setupCAN();
  
}

void loop() {
  if(last_send <= 950 + millis()){
    fetch();
    sender();
  }

  reader();
  delay(1);
}







/////////////////////////////////////////// F U N C T I O N S /////////////////////////////////////////////////

//-------SETUPS---------//
void setupCAN(){
  canMsgOUT.can_id = 0x1806E5F4 | CAN_EFF_FLAG;
  canMsgOUT.can_dlc = 8;
}
//------!SETUPS---------//


//-------CANNERS--------//
void sender(){
  Serial.println(mcp2515.TXB1,mcp2515.sendMessage(&canMsgOUT));
  last_send = millis();
}

void reader(){

}
//------!CANNERS--------//


//-------DATA-CALCS-----//
void fetch(){
  int phyPor = (100*(phyBatV - vBat_min) / (vBat_max - vBat_min));
  if(phyPor > 100){
    phyPor = 100;
  } else if (phyPor < 0){
    phyPor = 0;
  }

  int porLeft = 100 - phyPor;

  int askI = 0;

  if(GO){
    if(porLeft < rampaZone){
      askI = I_max * porLeft / rampaZone;
      if(askI<5){
        askI = 5;
      }
    } else {
      askI = I_max;
    }
  }
  
  
  
  int canVmax = vBat_max * 10;
  int canCurr_max = askI * 10;
  canMsgOUT.data[0] = highByte(canVmax);
  canMsgOUT.data[1] = lowByte(canVmax);
  canMsgOUT.data[2] = 0x10;
  canMsgOUT.data[3] = lowByte(canCurr_max);
  canMsgOUT.data[4] = 0x00;
  canMsgOUT.data[5] = canMsgOUT.data[5]++;
  canMsgOUT.data[6] = 0xFF;
  canMsgOUT.data[7] = 0xFF;
} 
//------!DATA-CALCS-----//