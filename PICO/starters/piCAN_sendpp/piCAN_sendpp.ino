#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif

//—————LIBS——————————————————————————————————————————————————————————————
#include <ACAN2515.h>


//--------pines mcp--------//
//static const byte MCP2515_INT  = 1; // INT output of MCP2515  //MCP2515 CON INT
static const byte MCP2515_INT = 255; // INT output of MCP2515  //MCP2515 SIN INT
static const byte MCP2515_SCK = 2;  // SCK input of MCP2515
static const byte MCP2515_MOSI = 3; // SDI input of MCP2515
static const byte MCP2515_MISO = 4; // SDO output of MCP2515
static const byte MCP2515_CS = 5;   // CS input of MCP2515

//--------variables CAN------//
ACAN2515 can(MCP2515_CS, SPI, MCP2515_INT);
CANMessage canMsg;
static const uint32_t QUARTZ_FREQUENCY = 8UL * 1000UL * 1000UL; // 8 MHz //  MCP2515 Quartz: adaptado al diseño



//-------------SETUP----------//
void setup() {
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);
  
  Serial.begin(38400);
  
  setupCAN();
  setupMssg55();
  delay(300);
}




void loop() {
  can.poll();
  const bool ok = can.tryToSend(canMsg);
  if (ok){
    Serial.println("sent");
  } else {
    Serial.println("try again");
  }
  delay(10);
}











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
  settings.mTransmitBuffer0Size = 30 ;
  //const uint16_t errorCode = can.begin(settings, [] {can.isr();}); //MCP2515 CON INT
  const uint16_t errorCode = can.begin(settings, NULL); // MCP2515 SIN INT

  if (errorCode == 0) {
    Serial.print ("Bit Rate prescaler: ") ;
    Serial.println (settings.mBitRatePrescaler) ;
    Serial.print ("Propagation Segment: ") ;
    Serial.println (settings.mPropagationSegment) ;
    Serial.print ("Phase segment 1: ") ;
    Serial.println (settings.mPhaseSegment1) ;
    Serial.print ("Phase segment 2: ") ;
    Serial.println (settings.mPhaseSegment2) ;
    Serial.print ("SJW: ") ;
    Serial.println (settings.mSJW) ;
    Serial.print ("Triple Sampling: ") ;
    Serial.println (settings.mTripleSampling ? "yes" : "no") ;
    Serial.print ("Actual bit rate: ") ;
    Serial.print (settings.actualBitRate ()) ;
    Serial.println (" bit/s") ;
    Serial.print ("Exact bit rate ? ") ;
    Serial.println (settings.exactBitRate () ? "yes" : "no") ;
    Serial.print ("Sample point: ") ;
    Serial.print (settings.samplePointFromBitStart ()) ;
    Serial.println ("%") ;
  }else{
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }

  canMsg.ext = false;
  canMsg.len = 8;
}


void setupMssg55(){
  canMsg.id = 0x055;
  
  int64_t phy_rpm = 0;
  float phy_torq = 0;


  int64_t hex_rpm = phy_rpm + 15000;
  canMsg.data[0] = highByte(hex_rpm);
  canMsg.data[1] = lowByte(hex_rpm);
  int64_t hex_torq = (phy_torq + 400)*5;
  canMsg.data[2] = ((hex_torq >> 4) & 0xF);
  canMsg.data[3] = (hex_torq & 0xF) << 4;
  canMsg.data[4] = 0x00;
  canMsg.data[5] = 0x00;
  canMsg.data[6] = (canMsg.data[6] + 1) & 0xF;
  
  byte checksum;
  checksum = canMsg.data[0] + canMsg.data[1] + canMsg.data[2] + canMsg.data[3] + canMsg.data[4] + canMsg.data[5] + canMsg.data[6];
  checksum = checksum ^ 0xFF;
  
  canMsg.data[7] = checksum;
}
