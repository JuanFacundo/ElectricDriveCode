#ifndef DA15_h
#define DA15_h
#include "Arduino.h"
#include <ACAN2515.h>
#include <SPI.h>

class DA15{
    private:
        //setup variables
        static const byte MCP2515_INT = 255; //sin INT
        static const byte MCP2515_SCK = 2;  // SCK input of MCP2515
        static const byte MCP2515_MOSI = 3; // SDI input of MCP2515
        static const byte MCP2515_MISO = 4; // SDO output of MCP2515
        static const byte MCP2515_CS = 5;
        static const uint32_t QUARTZ_FREQUENCY = 8UL * 1000UL * 1000UL;
        ACAN2515 can;
        ACAN2515Settings settings; // CAN bit rate 500 kb/s
        CANMessage canMsg81, canMsgMCU;

        //content variables

    public:
        DA15():ACAN2515(uint8_t, arduino::SPIClass&, uint8_t):ACAN2515Settings(uint32_t, uint32_t, uint32_t);

        void cansetup();

};

#endif