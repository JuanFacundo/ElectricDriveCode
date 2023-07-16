#include "Arduino.h"
#include "DA15.h"
#include <ACAN2515.h>
#include <SPI.h>

DA15::DA15():can(5,SPI,255):settings(QUARTZ_FREQUENCY, 500UL * 1000UL){
    cansetup();
}


void DA15::cansetup(){
    SPI.setSCK(2);
    SPI.setTX(3);
    SPI.setRX(4);
    SPI.setCS(5);
    //--- Begin SPI
    SPI.begin();
    //--- Configure ACAN2515
    settings.mRequestedMode = ACAN2515Settings::NormalMode;      // NormalMode (default value), ListenOnlyMode,LoopBackMode, SleepMode.
    // const uint16_t errorCode = can.begin(settings, [] {can.isr();}); //MCP2515 CON INT
    can.begin(settings, NULL); // MCP2515 SIN INT
}


