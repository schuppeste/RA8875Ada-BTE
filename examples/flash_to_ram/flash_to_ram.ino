#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"

#define RA8875_INT 2
#define RA8875_CS 4
#define RA8875_RESET 3
#define RA8875_FNCR0          0x21//Font Control Register 0
Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
char mychar[6] = "00:00";

void setup()
{
  Serial.begin(115200);
  Serial.println("RA8875 start");
  if (!tft.begin(RA8875_480x272)) {
    Serial.println("RA8875 Not Found!");
    while (1);
  }

  tft.displayOn(true);
  tft.GPIOX(true);      // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(255 );
  pinMode(RA8875_INT, INPUT);
  digitalWrite(RA8875_INT, HIGH);
  tft.touchEnable(false);
  tft.fillScreen(RA8875_BLACK);
  tft.graphicsMode();
  tft.Transparent_Mode();
  tft.writeTo(2);
  tft.fillScreen(RA8875_BLACK);
  tft.dispic(1);
  tft.dispicown(0, 0, 480, 232, 0); //Dispicown(startx,starty,width,height, Flashadress)
  //Copy to Blocktransfert to Screen startx,starty with*height @ startadress 
  //see Library files for more Information!!
}

void loop()
{
  delay(1000)
  //example Time Library:
  //sprintf(mychar, "%2.2d:%2.2d", hour(), minute()); //Anzeigeformat Uhrzeit
  //the trick, we sort Fontbitmaps in ascii-code order so we can calculate and access every char with:
  //first ascii char begins with ascii Code 33
  // So we calculate every char ->>>> Font-Startaddress+((asciiCode-33)*(FontWidth*fontHeight))
  
 // if (mychar[0] != oldtime[0])
 //   tft.dispicown(60, 65, 70, 119, (uint32_t)16660 + ((uint32_t)((uint32_t)mychar[0] - (uint32_t)33) * (uint32_t)16660));
  //if (mychar[1] != oldtime[1])
  //  tft.dispicown(130, 65, 70, 119, (uint32_t)16660 + ((uint32_t)((uint32_t)mychar[1] - (uint32_t)33) * (uint32_t)16660));
  //if (mychar[2] != oldtime[2])
  //  tft.dispicown(200, 65, 70, 119, (uint32_t)16660 + ((uint32_t)((uint32_t)mychar[2] - (uint32_t)33) * (uint32_t)16660));
  //if (mychar[3] != oldtime[3])
  //  tft.dispicown(270, 65, 70, 119, (uint32_t)16660 + ((uint32_t)((uint32_t)mychar[3] - (uint32_t)33) * (uint32_t)16660));
  //if (mychar[4] != oldtime[4])
  //  tft.dispicown(340, 65, 70, 119, (uint32_t)16660 + ((uint32_t)((uint32_t)mychar[4] - (uint32_t)33) * (uint32_t)16660));
}
