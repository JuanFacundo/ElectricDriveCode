#ifndef cansetup_h
#define cansetup_h
#include "Arduino.h"
#include <ACAN2515.h>


class cansetup{
    private:
        byte MCP2515_INT = 255; //sin INT
        byte MCP2515_SCK = 2;  // SCK input of MCP2515
        byte MCP2515_MOSI = 3; // SDI input of MCP2515
        byte MCP2515_MISO = 4; // SDO output of MCP2515
        byte MCP2515_CS = 5;
        static const uint32_t QUARTZ_FREQUENCY = 8UL * 1000UL * 1000UL;
    public:
        cansetup(byte sck, byte mosi, byte miso, byte cs);
        const ACAN2515 run(); 
};


#endif