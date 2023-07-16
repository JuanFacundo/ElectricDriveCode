#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsg1;
MCP2515 mcp2515(10);


void setup() {
  canMsg1.can_id  = 0x055;
  canMsg1.can_dlc = 8;
  canMsg1.data[0] = 0x5C;  //a partir de acá son
  canMsg1.data[1] = 0x19;  //todos los datos que se
  canMsg1.data[2] = 3;  //van a enviar.
  canMsg1.data[3] = 4;
  canMsg1.data[4] = 5;
  canMsg1.data[5] = 6;
  canMsg1.data[6] = 7;
  canMsg1.data[7] = 8;

  Serial.begin(9600);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_250KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  

}






void loop() {
  mcp2515.sendMessage(&canMsg1);
 
  Serial.println("mssg sent");
  delay(20);
}
