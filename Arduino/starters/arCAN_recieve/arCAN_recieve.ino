#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsg;
struct can_frame canMsg2;
MCP2515 mcp2515(10);


void setup() {
  canMsg2.can_id  = 0x047;
  canMsg2.can_dlc = 8;
  canMsg2.data[0] = 1;  //a partir de acá son
  canMsg2.data[1] = 1;  //todos los datos que se
  canMsg2.data[2] = 1;  //van a enviar.
  canMsg2.data[3] = 1;
  canMsg2.data[4] = 1;
  canMsg2.data[5] = 1;
  canMsg2.data[6] = 1;
  canMsg2.data[7] = 1;

  
  Serial.begin(115200);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_250KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();


  while(!Serial){
    delay(1);
  }
  Serial.println("------- CAN Read ----------");
}

void loop() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    Serial.print("ID: ");
    Serial.print(canMsg.can_id, HEX); // print ID
    //Serial.println();
    //Serial.print("DLC: "); 
    //Serial.print(canMsg.can_dlc); // print DLC
    //Serial.println();
    
    Serial.print(" DATA: ");
    for (int i = 0; i<canMsg.can_dlc; i++){
      if (canMsg.data[i]<16){
        Serial.print("0");
      }
      Serial.print(canMsg.data[i],HEX);
      Serial.print(" ");
    }

    Serial.println();
    //mcp2515.sendMessage(&canMsg2);
    //Serial.println("Messages sent");      
  }
  

  
}
