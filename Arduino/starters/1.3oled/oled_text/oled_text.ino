//A4 to SDA
//A5 to SCL 
// NOTE: You _must_ call display after making any drawing commands
// to make them visible on the display hardware! (display.clearDisplay();)

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#define OLED_RESET 4
Adafruit_SH1106 display(OLED_RESET);


void setup() {
  Serial.begin(9600);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Hello, world!!!!!!!!!!!!");
  display.setCursor(1,1);
  display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.println(3.141592);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print("0x"); display.println(0xDEADBEEF, HEX);
  display.display();

  display.clearDisplay();
  
}

void loop() {
  display.setCursor(0,0);//POSICION DE INICION DE ESCRITURA
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);
  display.println("OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO");//TEXTO EN PANTALLA
  display.println("OOOOOOOOOOOOOOOOOOOOO");
  display.setCursor(6,8);
  display.print(" ");
  display.display();
  display.setCursor(6,8);
  display.print("o");
  display.display();
  delay(1000);//TIEMPO DE ESPERA

}
