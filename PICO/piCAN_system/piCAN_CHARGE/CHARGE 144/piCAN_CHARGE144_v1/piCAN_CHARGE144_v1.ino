#ifndef ARDUINO_ARCH_RP2040 
#error "Select a Raspberry Pi Pico board"
#endif

//-------libs-----------//
#include <ACAN2515.h>
//-------/libs----------//

/////////////////////////////////////////////////////////////////GLOBALS/////////////////////////////////////////////////////////////////
//-------pins----------//
// static const byte MCP2515_INT  = 1; // INT output of MCP2515  //MCP2515 CON INT
static const byte MCP2515_INT = 255; // INT output of MCP2515  //MCP2515 SIN INT
static const byte MCP2515_SCK = 2;  // SCK input of MCP2515
static const byte MCP2515_MOSI = 3; // SDI input of MCP2515
static const byte MCP2515_MISO = 4; // SDO output of MCP2515
static const byte MCP2515_CS = 5;   // CS input of MCP2515
//------/pins-----------//

//-------can-variables--//
ACAN2515 can(MCP2515_CS, SPI, MCP2515_INT);
CANMessage canMsgOut, canMsgIn;
static const uint32_t QUARTZ_FREQUENCY = 8UL * 1000UL * 1000UL; // 8 MHz //  MCP2515 Quartz: adaptado al diseño
//------/can-variables--//



//-----------batery constants----------//
int n_cells = 48;
float vCell_max = 3.6;//3.34;
float vCell_min = 2.8;
float I_max = 20.0;                     //0.1 precision
float I_min = 1.0;

float vBat_max = vCell_max * n_cells;   //0.1 precision
float vBat_min = vCell_min * n_cells;   //0.1 precision
float rampaZone = 3.4;                  //donde las celdas empiezan a dispararse.
//----------/batery constants----------//



//-----------phy-values----------------//
float vChr = 0.0;
float Ichr = 0.0;
//-----------phy-values----------------//



//-------timers-------//
#define sendInterval 950
long lastSend = millis();
#define makeInterval 50
long lastMake = millis();
//------/timers-------//



//-------counters-&-flags-------//
bool msgEna = 1;
bool GO = 1;
int colorTime = 0;
//------/counters-&-flags-------//

/////////////////////////////////////////////////////////////////~GLOBALS/////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////MAINS/////////////////////////////////////////////////////////////////
void setup(){
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(38400);

  /*
  while(!Serial){
    delay(1);
  }//*/

  setupCAN();
  for (int i=0;i<10;i++){
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(100);
  }
}




void loop() {
  can.poll();
  CANreader();

  if(millis() >= lastMake + makeInterval){
    makePack();
  }
  
  if(millis() >= lastSend + sendInterval){
    CANsender();
  }

}


/////////////////////////////////////////////////////////////////~MAINS/////////////////////////////////////////////////////////////////







/////////////////////////////////////////////////////////////////FUNCTIONS/////////////////////////////////////////////////////////////////

//-------settings------//

void setupCAN(){
  SPI.setSCK(MCP2515_SCK);
  SPI.setTX(MCP2515_MOSI);
  SPI.setRX(MCP2515_MISO);
  SPI.setCS(MCP2515_CS);
  //--- Begin SPI
  SPI.begin();
  //--- Configure ACAN2515
  ACAN2515Settings settings(QUARTZ_FREQUENCY, 250UL * 1000UL); // CAN bit rate 250 kb/s
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

  //msgOUT ID
  canMsgOut.ext = true;
  canMsgOut.id = 0x18E54024;
  canMsgOut.len = 8;
}

//------/settings------//

//------canners-----//
void CANsender(){
  can.poll();
  if (msgEna == 0){
    const bool ok = can.tryToSend(canMsgOut);
    if (ok){
      lastSend = millis();
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, LOW);
      //Serial.println("tx full");
    }
  }
}

void CANreader(){
  can.poll();
  if(can.available()){
    can.receive(canMsgIn);
    /*
    if (canMsgIn.id == 0x18FEF4E5){
      //receiveFE();
      //printerFE();
    } //*/
    //*
    if (canMsgIn.id == 0x18EB2440){
      receive2440();
      printer2440();
      colorTime++;
    } //*/
    //*
    else {
      Serial.print("ID: ");
      Serial.print(canMsgIn.id, HEX); // print ID
    
    
      Serial.print(" DATA: ");
      for (int i = 0; i<canMsgIn.len; i++){
        if (canMsgIn.data[i]<16){
          Serial.print("0");
        }
        Serial.print(canMsgIn.data[i],HEX);
        Serial.print(" ");
      }

      Serial.println();
    } //*/
  }
}
//-----/canners-----//



//------data-calcs-----//
void makePack(){
  lastMake = millis();
  float askI = I_min;
  byte lightColor = 0x06; //rojo y verde
  if(GO){
    if(vChr <= rampaZone * n_cells){
      askI = I_max;
      lightColor = 0x04; //verde titila 
    } else if (vChr <= vCell_max * n_cells){
      askI = (pow(vChr-(vCell_max*n_cells),4) / (pow((rampaZone-vCell_max)*n_cells,4) / (I_max - I_min) )) + I_min;
      lightColor = Y_G(); //amarillo
    }
  }
  
  Serial.println(askI);
  int canVmax = vBat_max * 10;
  int canI_max = (-askI + 320) * 10;
  canMsgOut.data[0] = 0xFC;
  canMsgOut.data[1] = lowByte(canVmax);
  canMsgOut.data[2] = highByte(canVmax);
  canMsgOut.data[3] = lowByte(canI_max);
  canMsgOut.data[4] = highByte(canI_max);
  canMsgOut.data[5] = lightColor;
  canMsgOut.data[6] = 0xFF;
  canMsgOut.data[7] = 0xFF;
}


byte Y_G(){
  //Serial.println(colorTime);
  if(colorTime < 4){
    return 0x05;
  }
  if(colorTime < 8){
    return 0x03;
  }
  colorTime = 0;
  return 0x02;
}



//-----/data-calcs-----//




//------receivers------//
void receive2440(){
  msgEna = 0;
  int aux;

  aux = (canMsgIn.data[3] << 8);
  aux = aux | canMsgIn.data[2];
  vChr = (aux * 0.1);

  aux = (canMsgIn.data[5] << 8);
  aux = aux | (canMsgIn.data[4]);
  Ichr = ((aux * 0.1) - 319.7)*-1;
}

//-----/receivers------//




//------printers-------//
void printer2440(){
  Serial.print("ID: 0x18EB2440  chr Volt: ");
  Serial.print(vChr);
  Serial.print("  chr I: ");
  Serial.println(Ichr);
}
//-----/printers-------//
