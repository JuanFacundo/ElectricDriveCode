#include <SPI.h>
#include <mcp2515.h>
#define pot A0

struct can_frame canMsg;
struct can_frame canMsg1;
unsigned long pTime = millis();
int mem=-1;
MCP2515 mcp2515(10);


void setup() {
  canMsg1.can_id  = 0x037;
  canMsg1.can_dlc = 1;
  canMsg1.data[0] = 0;


  
  while (!Serial);
  Serial.begin(38400);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  
  Serial.println("Example: Write to CAN");
}


void loop() {
  unsigned long cTime = millis();

  //task 1: Reading (every 100ms)
  if (cTime - pTime >= 100){
    canMsg1.data[0]=map(analogRead(pot),0,1023,0,255); //leo y asigno
    if (canMsg1.data[0]!=mem){
      mcp2515.sendMessage(&canMsg1);
      mem=canMsg1.data[0];
    }
    

    
    pTime = cTime;
  }


  //task 2:reeding
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    Serial.print("ID: ");
    Serial.print(canMsg.can_id, HEX); // print ID
    Serial.println();
    Serial.print("DLC: "); 
    Serial.print(canMsg.can_dlc); // print DLC
    Serial.println();
    
    Serial.print("DATA: ");
    for (int i = 0; i<canMsg.can_dlc; i++){
      Serial.print(canMsg.data[i]);
      Serial.print(" ");
    }
    Serial.println();     
  }  

  
  
}
