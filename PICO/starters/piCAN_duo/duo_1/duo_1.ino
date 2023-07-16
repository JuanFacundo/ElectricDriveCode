#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif


#include <ACAN2515.h>


static const byte MCP2515_CS1 = 5; // CS input of MCP2515 CAN1
static const byte MCP2515_INT1 = 255 ; // INT output of MCP2515 CAN1
ACAN2515 can1(MCP2515_CS1, SPI, MCP2515_INT1);

static const byte MCP2515_CS2 = 9;//? // CS input of MCP2515 CAN2
static const byte MCP2515_INT2 = 255 ; // INT output of MCP2515 CAN2
ACAN2515 can2(MCP2515_CS2, SPI, MCP2515_INT2);

static const uint32_t QUARTZ_FREQUENCY = 16UL * 1000UL * 1000UL; // 16 MHz

void setup() {
  Serial.begin (9600);
  
  while (!Serial) {
    delay (50) ;
  }


  SPI.begin();
  ACAN2515Settings settings1 (QUARTZ_FREQUENCY, 500UL * 1000UL);
  settings1.mRequestedMode = ACAN2515Settings::NormalMode;
  can1.begin(settings1, NULL);
  /*const uint16_t errorCode1 = CAN1.begin (settings1, [] { CAN1.isr () ; }) ;
  if (errorCode1 != 0) {
    Serial.print ("Configuration CAN1 error 0x") ;
    Serial.println (errorCode1, HEX) ;
  } else {
    Serial.println ("Configuration CAN1 OK!") ;
  }*/

  ACAN2515Settings settings2(QUARTZ_FREQUENCY, 500UL * 1000UL);
  settings2.mRequestedMode = ACAN2515Settings::NormalMode;
  can2.begin(settings2, NULL);
  /*const uint16_t errorCode2 = CAN2.begin (settings2, [] { CAN2.isr () ; }) ;
  if (errorCode2 != 0) {
    Serial.print ("Configuration CAN2 error 0x") ;
    Serial.println (errorCode2, HEX) ;
  } else {
    Serial.println ("Configuration CAN2 OK!") ;
  }*/

}

void loop() {
  // put your main code here, to run repeatedly:

}
