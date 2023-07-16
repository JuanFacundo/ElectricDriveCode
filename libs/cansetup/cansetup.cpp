#include "Arduino.h"
#include "cansetup.h"
#include <ACAN2515.h>


cansetup::cansetup(byte sck, byte mosi, byte miso, byte cs){
    MCP2515_SCK = sck;  // SCK input of MCP2515
    MCP2515_MOSI = mosi; // SDI input of MCP2515
    MCP2515_MISO = miso; // SDO output of MCP2515
    MCP2515_CS = cs;
}

const ACAN2515 cansetup::run(){
    SPI.setSCK(MCP2515_SCK);
    SPI.setTX(MCP2515_MOSI);
    SPI.setRX(MCP2515_MISO);
    SPI.setCS(MCP2515_CS);
    //--- Begin SPI
    SPI.begin();
    //--- Configure ACAN2515
    ACAN2515Settings settings(QUARTZ_FREQUENCY, 500UL * 1000UL); // CAN bit rate 500 kb/s
    settings.mRequestedMode = ACAN2515Settings::NormalMode;      // NormalMode (default value), ListenOnlyMode,LoopBackMode, SleepMode.
    // const uint16_t errorCode = can.begin(settings, [] {can.isr();}); //MCP2515 CON INT
    ACAN2515 can(MCP2515_CS, SPI, MCP2515_INT);
    can.begin(settings, NULL); // MCP2515 SIN INT
    return can;
}









