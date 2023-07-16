#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif

//——————————————————————————————————————————————————————————————————————————————
#include <ACAN2515.h>
#include <SimpleTimer.h>
SimpleTimer trmTRAMA, trmENVIO; // crear objeto de temporizador
//——————————————————————————————————————————————————————————————————————————————
#define serialcan Serial
//------------------VARIABLES MCP -----------------------//
// static const byte MCP2515_INT  = 1; // INT output of MCP2515  //MCP2515 CON INT
static const byte MCP2515_INT = 255; // INT output of MCP2515  //MCP2515 SIN INT
static const byte MCP2515_SCK = D2;  // SCK input of MCP2515
static const byte MCP2515_MOSI = D3; // SDI input of MCP2515
static const byte MCP2515_MISO = D4; // SDO output of MCP2515
static const byte MCP2515_CS = D5;   // CS input of MCP2515
//------------------VARIABLES MCP -----------------------//

//------------------VARIABLES CAN -----------------------//
ACAN2515 can(MCP2515_CS, SPI, MCP2515_INT);
CANMessage canMsgRECIBO, canMsgENVIO;
static const uint32_t QUARTZ_FREQUENCY = 8UL * 1000UL * 1000UL; // 8 MHz //  MCP2515 Quartz: adaptado al diseño
//------------------VARIABLES CAN -----------------------//


static const uint32_t USBBaud = 38400;
static uint32_t retardoCAN1 = 0, retardoCAN2 = 0, retardoCAN3 = 0, retardoCAN4 = 0; // Tiempo que tarda cada envio
byte ENVIOUART = 0, TRAMAOK = 0;
// ENTRADAS DIGITALES
const int SENSOR_DIGITAL1 = D16;
const int SENSOR_DIGITAL2 = D17;
const int SENSOR_DIGITAL3 = D18;
const int SENSOR_DIGITAL4 = D19;
const int SENSOR_DIGITAL5 = D20;
const int SENSOR_DIGITAL6 = D21;
const int SENSOR_DIGITAL7 = D22;
const int SENSOR_DIGITAL8 = D6;

long ENVIOS = 0;
byte BYTE1 = 0x00, BYTE2 = 0x00, BYTE3 = 0x00, BYTE4 = 0x00,
     BYTE5 = 0x00, BYTE6 = 0x00, BYTE7 = 0x00, BYTE8 = 0x00,
     NROENVIOS = 0;

void setup()
{
  serialcan.begin(USBBaud);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  for (int i = 0; i < 20; i++)
  {
    delay(100);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
  digitalWrite(LED_BUILTIN, LOW);
  setupCAN();
  delay(300);
  setupSENSORES();
  delay(300);
  trmTRAMA.setInterval(200, tramaVCU1);
  trmENVIO.setInterval(50, enviarCAN);
}

void setupCAN()
{
  serialcan.println("------- SETUP CAN 1----------");
  SPI.setSCK(MCP2515_SCK);
  SPI.setTX(MCP2515_MOSI);
  SPI.setRX(MCP2515_MISO);
  SPI.setCS(MCP2515_CS);
  //--- Begin SPI
  SPI.begin();
  //--- Configure ACAN2515
  serialcan.println("Configure ACAN2515");
  ACAN2515Settings settings(QUARTZ_FREQUENCY, 500UL * 1000UL); // CAN bit rate 500 kb/s
  settings.mRequestedMode = ACAN2515Settings::NormalMode;      // NormalMode (default value), ListenOnlyMode,LoopBackMode, SleepMode.
  // const uint16_t errorCode = can.begin(settings, [] {can.isr();}); //MCP2515 CON INT
  const uint16_t errorCode = can.begin(settings, NULL); // MCP2515 SIN INT
  if (errorCode == 0)
  {
    serialcan.println("CAN INICIALIZACION CORRECTA!!!");
    serialcan.print("mBitRatePrescaler: ");
    serialcan.println(settings.mBitRatePrescaler);
    serialcan.print("mPropagationSegment: ");
    serialcan.println(settings.mPropagationSegment);
    serialcan.print("mPhaseSegment1: ");
    serialcan.println(settings.mPhaseSegment1);
    serialcan.print("mPhaseSegment2: ");
    serialcan.println(settings.mPhaseSegment2);
    serialcan.print("mSJW: ");
    serialcan.println(settings.mSJW);
    serialcan.print("mTripleSampling: ");
    serialcan.println(settings.mTripleSampling ? "SI" : "NO");
    serialcan.print("actualBitRate: ");
    serialcan.print(settings.actualBitRate());
    serialcan.println(" bit/s");
    serialcan.print("exactBitRate ? ");
    serialcan.println(settings.exactBitRate() ? "SI" : "NO");
    serialcan.print("samplePointFromBitStart: ");
    serialcan.println(settings.samplePointFromBitStart());
    serialcan.print("mBitRateClosedToDesiredRate: ");
    serialcan.println(settings.mBitRateClosedToDesiredRate); // 1 (--> is true)
    serialcan.print("CANBitSettingConsistency: 0x % ");
    serialcan.println(settings.CANBitSettingConsistency(), HEX); // 0x10, meaning error
    serialcan.print("receiveBufferSize: ");
    serialcan.println(settings.mReceiveBufferSize);
  }
  else
  {
    serialcan.print("CAN ERROR CONFIGURACION 0x");
    serialcan.println(errorCode, HEX);
  }
  serialcan.println("------- SETUP CAN 2----------");
}

void setupSENSORES()
{
  serialcan.println("------- SETUP SENSORES----------");
  // Lectura de entradas
  pinMode(SENSOR_DIGITAL1, INPUT);
  pinMode(SENSOR_DIGITAL2, INPUT);
  pinMode(SENSOR_DIGITAL3, INPUT);
  pinMode(SENSOR_DIGITAL4, INPUT);
  pinMode(SENSOR_DIGITAL5, INPUT);
  pinMode(SENSOR_DIGITAL6, INPUT);
  pinMode(SENSOR_DIGITAL7, INPUT);
  pinMode(SENSOR_DIGITAL8, INPUT);
}

byte rcounter = 0;
void tramaVCU1()
{
  canMsgENVIO.id = 0x81;
  canMsgENVIO.len = 8;
  canMsgENVIO.ext = false;
  canMsgENVIO.data[0] = BYTE1;
  canMsgENVIO.data[1] = BYTE2;
  canMsgENVIO.data[2] = BYTE3;
  canMsgENVIO.data[3] = BYTE4;
  canMsgENVIO.data[4] = BYTE5;
  canMsgENVIO.data[5] = BYTE6;
  if (rcounter < 16)
  {
    rcounter++;
  }
  else
  {
    rcounter = 0;
  }
  canMsgENVIO.data[6] = rcounter;
  BYTE8 = XORChecksum8(6);
  canMsgENVIO.data[7] = BYTE8;
} // FIN tramaVCU1

void loop()
{
  trmTRAMA.run();
  trmENVIO.run();
  recibirCMD();
  recibirCAN();
} // FIN loop

void recibirCMD()
{
  float phy_torq = map(analogRead(26),0,1023,-51,10); //modif max
  float phy_rpm = 0;
  bool anslo = 0;
  byte gear = readGear();

  bool v2m = 1;
  byte mmode;
  if(digitalRead(6) == LOW){
    mmode=2;
  }
  else mmode=1;
  

  byte checksum;  
  int can_torq = (phy_torq + 400) * 5;
  int can_speed = (phy_rpm + 15000);
  ENVIOS=0;
  BYTE1 = byte0(can_torq);
  BYTE2 = byte1(can_torq,anslo,gear);
  BYTE3 = byte2(can_speed);
  BYTE4 = byte3(can_speed);
  BYTE5 = byte4(v2m,mmode);
  BYTE6 = 0;
  //canMsg.data[6] = byte6(canMsg.data[6]);

  //checksum = canMsg.data[0] + canMsg.data[1] + canMsg.data[2] + canMsg.data[3] + canMsg.data[4] + canMsg.data[5] + canMsg.data[6];
  //checksum = checksum ^ 0xFF;
  
  //canMsg.data[7] = checksum;
} // FIN recibirCMD

byte readGear(){
  for (int i=0;i<6;i++){
    if (digitalRead(i+16) == LOW){
      return i;
    }
  }
  return 0;
}

byte byte0(int torq){
  return (torq >> 4);
}


byte byte1(int torq, bool anslo, byte gear){
  byte b_1 = torq;
  b_1 = (b_1 << 4);
  
  if (anslo){
    b_1 = (b_1 | 8);
  }

  b_1 = (b_1 | gear);

  return b_1;
}

byte byte2(int rpm){
  return (rpm >> 8);
}

byte byte3(int rpm){
  return rpm;
}

byte byte4(bool v2m, byte mmode){
  byte b_4 = mmode;

  if (v2m){
    b_4 = (b_4 | 16);
  }

  return b_4;
}

byte byte6(byte count){
  count++;
  return (15 & count);
}








void recibirCAN()
{
//  can.poll(); // MCP2515 SIN INT
} // FIN recibirCAN






void enviarCAN()
{
  if (ENVIOS <= NROENVIOS || NROENVIOS == 99)
  {
    ENVIOS++;
    bool okEnvio = can.tryToSend(canMsgENVIO);
    verTramaEnviada(okEnvio);
  }
} // FIN enviarCAN

void verTramaRecibida(bool resultado)
{
  serialcan.println();
  serialcan.print("RECIBO_ID:");
  serialcan.print(canMsgRECIBO.id, HEX);
  serialcan.print(" ");
  for (int i = 0; i < canMsgRECIBO.len; i++)
  { // print the data
    if (canMsgRECIBO.data[i] < 16)
    {
      serialcan.print("0");
      serialcan.print(canMsgRECIBO.data[i], HEX);
    }
    else
    {
      serialcan.print(canMsgRECIBO.data[i], HEX);
    }
    serialcan.print(" ");
  }
  if (resultado)
  {
    serialcan.print("OK");
  }
  else
  {
    serialcan.print("???");
  }
}

void verTramaEnviada(bool resultado)
{
  serialcan.println();
  serialcan.print("ENVIO_ID:");
  serialcan.print(canMsgENVIO.id, HEX);
  serialcan.print(" ");
  for (int i = 0; i < canMsgENVIO.len; i++)
  { // print the data
    if (canMsgENVIO.data[i] < 16)
    {
      serialcan.print("0");
      serialcan.print(canMsgENVIO.data[i], HEX);
    }
    else
    {
      serialcan.print(canMsgENVIO.data[i], HEX);
    }
    serialcan.print(" ");
  }
  if (resultado)
  {
    serialcan.print("OK ");
  }
  else
  {
    serialcan.print("NOOK ");
  }
  serialcan.print(ENVIOS);
}

void fnStrToBytes(byte *byteArray, const char *hexString)
{
  bool oddLength = strlen(hexString) & 1;
  byte currentByte = 0;
  byte byteIndex = 0;
  for (byte charIndex = 0; charIndex < strlen(hexString); charIndex++)
  {
    bool oddCharIndex = charIndex & 1;
    if (oddLength)
    {
      // If the length is odd
      if (oddCharIndex)
      {
        // odd characters go in high nibble
        currentByte = fnNibble(hexString[charIndex]) << 4;
      }
      else
      {
        // Even characters go into low nibble
        currentByte |= fnNibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
    else
    {
      // If the length is even
      if (!oddCharIndex)
      {
        // Odd characters go into the high nibble
        currentByte = fnNibble(hexString[charIndex]) << 4;
      }
      else
      {
        // Odd characters go into low nibble
        currentByte |= fnNibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
  }
}

void fnVerArreglo(const byte *byteArray, const byte arraySize)
{
  Serial.println();
  for (int i = 0; i < arraySize; i++)
  {
    Serial.print("0x");
    if (byteArray[i] < 0x10)
      Serial.print("0");
    Serial.print(byteArray[i], HEX);
    Serial.print(" ");
  }
}

byte fnNibble(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';

  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  return 0; // Not a valid hexadecimal character
}

uint8_t XORChecksum8(size_t dataLength)
{
  uint16_t value = 0;
  for (size_t i = 0; i <= dataLength; i++)
  {
    value += (uint8_t)canMsgENVIO.data[i];
  }
  value ^= 0xFF;
  return (uint8_t)value;
}
