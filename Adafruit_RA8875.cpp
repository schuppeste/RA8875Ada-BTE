/**************************************************************************/
/*!
    @file     Adafruit_RA8875.cpp
    @author   Limor Friend/Ladyada, K.Townsend/KTOWN for Adafruit Industries
    @license  BSD license, all text above and below must be included in
              any redistribution

 This is the library for the Adafruit RA8875 Driver board for TFT displays
 ---------------> http://www.adafruit.com/products/1590
 The RA8875 is a TFT driver for up to 800x480 dotclock'd displays
 It is tested to work with displays in the Adafruit shop. Other displays
 may need timing adjustments and are not guanteed to work.
 
 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source hardware
 by purchasing products from Adafruit!
 
 Written by Limor Fried/Ladyada for Adafruit Industries.
 BSD license, check license.txt for more information.
 All text above must be included in any redistribution.

    @section  HISTORY
    
    v1.0 - First release
*/
/**************************************************************************/
#include <SPI.h>
#include "Adafruit_RA8875.h"

/**************************************************************************/
/*!
      Constructor for a new RA8875 instance
      
      @args CS[in]  Location of the SPI chip select pin
      @args RST[in] Location of the reset pin
*/
/**************************************************************************/
Adafruit_RA8875::Adafruit_RA8875(uint8_t CS, uint8_t RST) : Adafruit_GFX(800, 480) {
  _cs = CS;
  _rst = RST;
}

/**************************************************************************/
/*!
      Initialises the LCD driver and any HW required by the display
      
      @args s[in] The display size, which can be either:
                  'RA8875_480x272' (4.3" displays) r
                  'RA8875_800x480' (5" and 7" displays)
*/
/**************************************************************************/
boolean Adafruit_RA8875::begin(enum RA8875sizes s) {
  _size = s;
_errorCode = 0;
	_size = s;
	_rotation = 0;
	_portrait = false;
	_inited = false;
	_sleep = false;
	_hasLayerLimits = false;
	_maxLayers = 2;
	_currentLayer = 1;
	_useMultiLayers = true;//starts with one layer only
	_currentMode = false;
	_brightness = 255;
	_cursorX = 0; _cursorY = 0; _scrollXL = 0; _scrollXR = 0; _scrollYT = 0; _scrollYB = 0;
	

  if (_size == RA8875_480x272) {
    _width = 480;
    _height = 272;
  } 
  if (_size == RA8875_800x480) {
    _width = 800;
    _height = 480;
  }
	_currentLayer = 0;
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);
  pinMode(_rst, OUTPUT); 
  digitalWrite(_rst, LOW);

  digitalWrite(_rst, LOW);
  delay(100);
  digitalWrite(_rst, HIGH);
  delay(100);
  
  SPI.begin();
#ifdef __AVR__
  SPI.setClockDivider(SPI_CLOCK_DIV128);
  SPI.setDataMode(SPI_MODE0);
#endif
  
  if (readReg(0) != 0x75) {
    return false;
  }

  initialize();

#ifdef __AVR__
  SPI.setClockDivider(SPI_CLOCK_DIV4);
#endif
  return true;
}

/************************* Initialization *********************************/

/**************************************************************************/
/*!
      Performs a SW-based reset of the RA8875
*/
/**************************************************************************/
void Adafruit_RA8875::softReset(void) {
  writeCommand(RA8875_PWRR);
  writeData(RA8875_PWRR_SOFTRESET);
  writeData(RA8875_PWRR_NORMAL);
  delay(1);
}
int next=0;
/**************************************************************************/
/*!
      Initialise the PLL
*/
/**************************************************************************/
void Adafruit_RA8875::PLLinit(void) {
  if (_size == RA8875_480x272) {
    writeReg(RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 10);
    delay(1);
    writeReg(RA8875_PLLC2, RA8875_PLLC2_DIV4);
    delay(1);
  }
  if (_size == RA8875_800x480) {
    writeReg(RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 10);
    delay(1);
    writeReg(RA8875_PLLC2, RA8875_PLLC2_DIV4);
    delay(1);
  }
}

/**************************************************************************/
/*!
      Initialises the driver IC (clock setup, etc.)
*/
/**************************************************************************/
void Adafruit_RA8875::initialize(void) {
  PLLinit();
  writeReg(RA8875_SYSR, RA8875_SYSR_16BPP | RA8875_SYSR_MCU8);

  /* Timing values */
  uint8_t pixclk;
  uint8_t hsync_start;
  uint8_t hsync_pw;
  uint8_t hsync_finetune;
  uint8_t hsync_nondisp;
  uint8_t vsync_pw; 
  uint16_t vsync_nondisp;
  uint16_t vsync_start;

  /* Set the correct values for the display being used */  
  if (_size == RA8875_480x272) 
  {
    pixclk          = RA8875_PCSR_PDATL | RA8875_PCSR_4CLK;
    hsync_nondisp   = 10;
    hsync_start     = 8;
    hsync_pw        = 48;
    hsync_finetune  = 0;
    vsync_nondisp   = 3;
    vsync_start     = 8;
    vsync_pw        = 10;
  } 
  else if (_size == RA8875_800x480) 
  {
    pixclk          = RA8875_PCSR_PDATL | RA8875_PCSR_2CLK;
    hsync_nondisp   = 26;
    hsync_start     = 32;
    hsync_pw        = 96;
    hsync_finetune  = 0;
    vsync_nondisp   = 32;
    vsync_start     = 23;
    vsync_pw        = 2;
  }

  writeReg(RA8875_PCSR, pixclk);
  delay(1);
  
  /* Horizontal settings registers */
  writeReg(RA8875_HDWR, (_width / 8) - 1);                          // H width: (HDWR + 1) * 8 = 480
  writeReg(RA8875_HNDFTR, RA8875_HNDFTR_DE_HIGH + hsync_finetune);
  writeReg(RA8875_HNDR, (hsync_nondisp - hsync_finetune - 2)/8);    // H non-display: HNDR * 8 + HNDFTR + 2 = 10
  writeReg(RA8875_HSTR, hsync_start/8 - 1);                         // Hsync start: (HSTR + 1)*8 
  writeReg(RA8875_HPWR, RA8875_HPWR_LOW + (hsync_pw/8 - 1));        // HSync pulse width = (HPWR+1) * 8
  
  /* Vertical settings registers */
  writeReg(RA8875_VDHR0, (uint16_t)(_height - 1) & 0xFF);
  writeReg(RA8875_VDHR1, (uint16_t)(_height - 1) >> 8);
  writeReg(RA8875_VNDR0, vsync_nondisp-1);                          // V non-display period = VNDR + 1
  writeReg(RA8875_VNDR1, vsync_nondisp >> 8);
  writeReg(RA8875_VSTR0, vsync_start-1);                            // Vsync start position = VSTR + 1
  writeReg(RA8875_VSTR1, vsync_start >> 8);
  writeReg(RA8875_VPWR, RA8875_VPWR_LOW + vsync_pw - 1);            // Vsync pulse width = VPWR + 1
  
  /* Set active window X */
  writeReg(RA8875_HSAW0, 0);                                        // horizontal start point
  writeReg(RA8875_HSAW1, 0);
  writeReg(RA8875_HEAW0, (uint16_t)(_width - 1) & 0xFF);            // horizontal end point
  writeReg(RA8875_HEAW1, (uint16_t)(_width - 1) >> 8);
  
  /* Set active window Y */
  writeReg(RA8875_VSAW0, 0);                                        // vertical start point
  writeReg(RA8875_VSAW1, 0);  
  writeReg(RA8875_VEAW0, (uint16_t)(_height - 1) & 0xFF);           // horizontal end point
  writeReg(RA8875_VEAW1, (uint16_t)(_height - 1) >> 8);
  
  /* ToDo: Setup touch panel? */
  
  /* Clear the entire window */
  writeReg(RA8875_MCLR, RA8875_MCLR_START | RA8875_MCLR_FULL);
  delay(500); 
}
//unsigned char code  pic[31360];
#define color_brown   0x40c0
#define color_black   0x0000
#define color_white   0xffff
#define color_red     0xf800
#define color_green   0x07e0
#define color_blue    0x001f
#define color_yellow  color_red|color_green
#define color_cyan    color_green|color_blue
#define color_purple  color_red|color_blue
/**************************************************************************/
/*!
      Returns the display width in pixels
      
      @returns  The 1-based display width in pixels
*/
/**************************************************************************/
uint16_t Adafruit_RA8875::width(void) { return _width; }

/**************************************************************************/
/*!
      Returns the display height in pixels

      @returns  The 1-based display height in pixels
*/
/**************************************************************************/
uint16_t Adafruit_RA8875::height(void) { return _height; }

/************************* Text Mode ***********************************/

/**************************************************************************/
/*!
      Sets the display in text mode (as opposed to graphics mode)
*/
/**************************************************************************/
void Adafruit_RA8875::Text_Foreground_Color1(uint b_color)
{
	
	writeCommand(0x63);//BGCR0
	writeData((uchar)(b_color>>11));
	
	writeCommand(0x64);//BGCR0
	writeData((uchar)(b_color>>5));
	
	writeCommand(0x65);//BGCR0
	writeData((uchar)(b_color));
} 
void Adafruit_RA8875::Text_Background_Color1(uint b_color)
{
	
	writeCommand(0x60);//BGCR0
	writeData((uchar)(b_color>>11));
	
	writeCommand(0x61);//BGCR0
	writeData((uchar)(b_color>>5));
	
	writeCommand(0x62);//BGCR0
	writeData((uchar)(b_color));
} 
void Adafruit_RA8875::textMode(void) 
{
  /* Set text mode 
  
			Serial.println("test");

			/////External characters of the functional test
		    Text_Foreground_Color1(color_white);//Set the foreground color
			Text_Background_Color1(color_black);//Set the background color		
			Active_Window(0,479,0,271);//Set the work window size
			Write_Dir(0X8E,0X80);//Start screen clearing (display window)
		    Chk_Busy();
			Write_Dir(0x21,0x20);//Select the external character
			Write_Dir(0x06,0x03);//Set the frequency
			Write_Dir(0x2E,0x80);//Font Write Type Setting Register Set up 24 x24 character mode     spacing   0 
			Write_Dir(0x2F,0x81);//Serial Font ROM Setting GT23L32S4W
			Write_Dir(0x05,0x28);// The waveform 3   2 byte dummy Cycle) 
			  */
/*
			FontWrite_Position(40,35);//Text written to the position
		    Write_Dir(0x40,0x80);//Set the character mode
		    writeCommand(0x02);//start write data
		    String("深圳旭日东方科技有限公司");
		
			Text_Foreground_Color1(color_red);//Set the foreground color
			Write_Dir(0x2E,0x01);//Set the characters mode 16 x16 / spacing 1
		    FontWrite_Position(100,80);//Text written to the position
			String("TEL:755-33503874 FAX:755-33507642");
			FontWrite_Position(100,96);//Text written to the position
			String("WWW.BUY-DISPLAY.COM");
			FontWrite_Position(100,112);//Text written to the position
			String("E-mail:market@lcd-china.com");
			FontWrite_Position(100,128);//Text written to the position
			String("AD:Room 6G,Building A1,Zhujiang Square,Zhongxin Cheng,Longgang District,ShenZhen,China");
*/
		

	
}

/**************************************************************************/
/*!
      Sets the display in text mode (as opposed to graphics mode)
      
      @args x[in] The x position of the cursor (in pixels, 0..1023)
      @args y[in] The y position of the cursor (in pixels, 0..511)
*/
/**************************************************************************/
void Adafruit_RA8875::textSetCursor(uint16_t x, uint16_t y) 
{
  /* Set cursor location */
  writeCommand(0x2A);
  writeData(x & 0xFF);
  writeCommand(0x2B);
  writeData(x >> 8);
  writeCommand(0x2C);
  writeData(y & 0xFF);
  writeCommand(0x2D);
  writeData(y >> 8);
}
void Adafruit_RA8875::FontWrite_Position(uint16_t x, uint16_t y) 
{
  /* Set cursor location */
  writeCommand(0x2A);
  writeData(x & 0xFF);
  writeCommand(0x2B);
  writeData(x >> 8);
  writeCommand(0x2C);
  writeData(y & 0xFF);
  writeCommand(0x2D);
  writeData(y >> 8);
}

/**************************************************************************/
/*!
      Sets the fore and background color when rendering text
      
      @args foreColor[in] The RGB565 color to use when rendering the text
      @args bgColor[in]   The RGB565 colot to use for the background
*/
/**************************************************************************/
void Adafruit_RA8875::textColor(uint16_t foreColor, uint16_t bgColor)
{
  /* Set Fore Color */
  writeCommand(0x63);
  writeData((foreColor & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((foreColor & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((foreColor & 0x001f));
  
  /* Set Background Color */
  writeCommand(0x60);
  writeData((bgColor & 0xf800) >> 11);
  writeCommand(0x61);
  writeData((bgColor & 0x07e0) >> 5);
  writeCommand(0x62);
  writeData((bgColor & 0x001f));
  
  /* Clear transparency flag */
  writeCommand(0x22);
  uint8_t temp = readData();
  temp &= ~(1<<6); // Clear bit 6
  writeData(temp);
}

/**************************************************************************/
/*!
      Sets the fore color when rendering text with a transparent bg
      
      @args foreColor[in] The RGB565 color to use when rendering the text
*/
/**************************************************************************/
void Adafruit_RA8875::textTransparent(uint16_t foreColor)
{
  /* Set Fore Color */
  writeCommand(0x63);
  writeData((foreColor & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((foreColor & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((foreColor & 0x001f));

  /* Set transparency flag */
  writeCommand(0x22);
  uint8_t temp = readData();
  temp |= (1<<6); // Set bit 6
  writeData(temp);  
}

/**************************************************************************/
/*!
      Sets the text enlarge settings, using one of the following values:
      
      0 = 1x zoom
      1 = 2x zoom
      2 = 3x zoom
      3 = 4x zoom
      
      @args scale[in]   The zoom factor (0..3 for 1-4x zoom)
*/
/**************************************************************************/
void Adafruit_RA8875::textEnlarge(uint8_t scale)
{
  if (scale > 3) scale = 3;

  /* Set font size flags */
  writeCommand(0x22);
  uint8_t temp = readData();
  temp &= ~(0xF); // Clears bits 0..3
  temp |= scale << 2;
  temp |= scale;
  writeData(temp);  

  _textScale = scale;
}

/**************************************************************************/
/*!
      Renders some text on the screen when in text mode
      
      @args buffer[in]    The buffer containing the characters to render
      @args len[in]       The size of the buffer in bytes
*/
/**************************************************************************/
void Adafruit_RA8875::Write_Dir(uchar Cmd,uchar Data)
{
  writeCommand(Cmd);
  writeData(Data);
}
 void Adafruit_RA8875::String(const char *str)
{   
    Write_Dir(0x40,0x80);//Set the character mode
	writeCommand(0x02);
	while(*str != '\0')
	{
	 writeData(*str);
	 ++str;	 	
		 delay(10);
	} 
}
void Adafruit_RA8875::textWrite(const char* buffer, uint16_t len) 
{
  if (len == 0) len = strlen(buffer);
  writeCommand(RA8875_MRWC);
  for (uint16_t i=0;i<len;i++)
  {
    writeData(buffer[i]);


  }
}

/************************* Graphics ***********************************/

/**************************************************************************/
/*!
      Sets the display in graphics mode (as opposed to text mode)
*/
/**************************************************************************/
void Adafruit_RA8875::graphicsMode(void) {
  writeCommand(RA8875_MWCR0);
  uint8_t temp = readData();
  temp &= ~RA8875_MWCR0_TXTMODE; // bit #7
  writeData(temp);
}

/**************************************************************************/
/*!
      Waits for screen to finish by polling the status!
*/
/**************************************************************************/
boolean Adafruit_RA8875::waitPoll(uint8_t regname, uint8_t waitflag) {
  /* Wait for the command to finish */
  long timeout=millis();
  while (1)
  {
    uint8_t temp = readReg(regname);
    if (!(temp & waitflag))
      return true;
  if ((millis() - timeout) > 20) return false;
  }  
  return false; // MEMEFIX: yeah i know, unreached! - add timeout?
}
boolean Adafruit_RA8875::waitPollvar(uint8_t regname, uint8_t waitflag) {
  /* Wait for the command to finish */
  
    uint8_t temp = readReg(regname);
    if (!(temp & waitflag)){
      return true;
	}
 else{
  return false; // MEMEFIX: yeah i know, unreached! - add timeout?
}
  }

/**************************************************************************/
/*!
      Sets the current X/Y position on the display before drawing
      
      @args x[in] The 0-based x location
      @args y[in] The 0-base y location
*/
/**************************************************************************/
void Adafruit_RA8875::setXY(uint16_t x, uint16_t y) {
  writeReg(RA8875_CURH0, x);
  writeReg(RA8875_CURH1, x >> 8);
  writeReg(RA8875_CURV0, y);
  writeReg(RA8875_CURV1, y >> 8);  
}

/**************************************************************************/
/*!
      HW accelerated function to push a chunk of raw pixel data
      
      @args num[in] The number of pixels to push
      @args p[in]   The pixel color to use
*/
/**************************************************************************/
void Adafruit_RA8875::pushPixels(uint32_t num, uint16_t p) {
  digitalWrite(_cs, LOW);
  SPI.transfer(RA8875_DATAWRITE);
  while (num--) {
    SPI.transfer(p >> 8);
    SPI.transfer(p);
  }
  digitalWrite(_cs, HIGH);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void Adafruit_RA8875::fillRect(void) {
  writeCommand(RA8875_DCR);
  writeData(RA8875_DCR_LINESQUTRI_STOP | RA8875_DCR_DRAWSQUARE);
  writeData(RA8875_DCR_LINESQUTRI_START | RA8875_DCR_FILL | RA8875_DCR_DRAWSQUARE);
}

/**************************************************************************/
/*!
      Draws a single pixel at the specified location

      @args x[in]     The 0-based x location
      @args y[in]     The 0-base y location
      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void Adafruit_RA8875::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  writeReg(RA8875_CURH0, x);
  writeReg(RA8875_CURH1, x >> 8);
  writeReg(RA8875_CURV0, y);
  writeReg(RA8875_CURV1, y >> 8);  
  writeCommand(RA8875_MRWC);
  digitalWrite(_cs, LOW);
  SPI.transfer(RA8875_DATAWRITE);
  SPI.transfer(color >> 8);
  SPI.transfer(color);
  digitalWrite(_cs, HIGH);
}

/**************************************************************************/
/*!
      Draws a HW accelerated line on the display
    
      @args x0[in]    The 0-based starting x location
      @args y0[in]    The 0-base starting y location
      @args x1[in]    The 0-based ending x location
      @args y1[in]    The 0-base ending y location
      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void Adafruit_RA8875::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  /* Set X */
  writeCommand(0x91);
  writeData(x0);
  writeCommand(0x92);
  writeData(x0 >> 8);
  
  /* Set Y */
  writeCommand(0x93);
  writeData(y0); 
  writeCommand(0x94);
  writeData(y0 >> 8);
  
  /* Set X1 */
  writeCommand(0x95);
  writeData(x1);
  writeCommand(0x96);
  writeData((x1) >> 8);
  
  /* Set Y1 */
  writeCommand(0x97);
  writeData(y1); 
  writeCommand(0x98);
  writeData((y1) >> 8);
  
  /* Set Color */
  writeCommand(0x63);
  writeData((color & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((color & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((color & 0x001f));

  /* Draw! */
  writeCommand(RA8875_DCR);
  writeData(0x80);
  
  /* Wait for the command to finish */
  waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void Adafruit_RA8875::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
  drawLine(x, y, x, y+h, color);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void Adafruit_RA8875::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
  drawLine(x, y, x+w, y, color);
}

/**************************************************************************/
/*!
      Draws a HW accelerated rectangle on the display

      @args x[in]     The 0-based x location of the top-right corner
      @args y[in]     The 0-based y location of the top-right corner
      @args w[in]     The rectangle width
      @args h[in]     The rectangle height
      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void Adafruit_RA8875::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  rectHelper(x, y, x+w, y+h, color, false);
}

/**************************************************************************/
/*!
      Draws a HW accelerated filled rectangle on the display

      @args x[in]     The 0-based x location of the top-right corner
      @args y[in]     The 0-based y location of the top-right corner
      @args w[in]     The rectangle width
      @args h[in]     The rectangle height
      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void Adafruit_RA8875::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  rectHelper(x, y, x+w, y+h, color, true);
}

/**************************************************************************/
/*!
      Fills the screen with the spefied RGB565 color

      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void Adafruit_RA8875::fillScreen(uint16_t color)
{  
  rectHelper(0, 0, _width-1, _height-1, color, true);
}

/**************************************************************************/
/*!
      Draws a HW accelerated circle on the display

      @args x[in]     The 0-based x location of the center of the circle
      @args y[in]     The 0-based y location of the center of the circle
      @args w[in]     The circle's radius
      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void Adafruit_RA8875::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
  circleHelper(x0, y0, r, color, false);
}

/**************************************************************************/
/*!
      Draws a HW accelerated filled circle on the display

      @args x[in]     The 0-based x location of the center of the circle
      @args y[in]     The 0-based y location of the center of the circle
      @args w[in]     The circle's radius
      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void Adafruit_RA8875::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
  circleHelper(x0, y0, r, color, true);
}

/**************************************************************************/
/*!
      Draws a HW accelerated triangle on the display

      @args x0[in]    The 0-based x location of point 0 on the triangle
      @args y0[in]    The 0-based y location of point 0 on the triangle
      @args x1[in]    The 0-based x location of point 1 on the triangle
      @args y1[in]    The 0-based y location of point 1 on the triangle
      @args x2[in]    The 0-based x location of point 2 on the triangle
      @args y2[in]    The 0-based y location of point 2 on the triangle
      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void Adafruit_RA8875::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  triangleHelper(x0, y0, x1, y1, x2, y2, color, false);
}

/**************************************************************************/
/*!
      Draws a HW accelerated filled triangle on the display

      @args x0[in]    The 0-based x location of point 0 on the triangle
      @args y0[in]    The 0-based y location of point 0 on the triangle
      @args x1[in]    The 0-based x location of point 1 on the triangle
      @args y1[in]    The 0-based y location of point 1 on the triangle
      @args x2[in]    The 0-based x location of point 2 on the triangle
      @args y2[in]    The 0-based y location of point 2 on the triangle
      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void Adafruit_RA8875::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  triangleHelper(x0, y0, x1, y1, x2, y2, color, true);
}

/**************************************************************************/
/*!
      Draws a HW accelerated ellipse on the display

      @args xCenter[in]   The 0-based x location of the ellipse's center
      @args yCenter[in]   The 0-based y location of the ellipse's center
      @args longAxis[in]  The size in pixels of the ellipse's long axis
      @args shortAxis[in] The size in pixels of the ellipse's short axis
      @args color[in]     The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void Adafruit_RA8875::drawEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color)
{
  ellipseHelper(xCenter, yCenter, longAxis, shortAxis, color, false);
}

/**************************************************************************/
/*!
      Draws a HW accelerated filled ellipse on the display

      @args xCenter[in]   The 0-based x location of the ellipse's center
      @args yCenter[in]   The 0-based y location of the ellipse's center
      @args longAxis[in]  The size in pixels of the ellipse's long axis
      @args shortAxis[in] The size in pixels of the ellipse's short axis
      @args color[in]     The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void Adafruit_RA8875::fillEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color)
{
  ellipseHelper(xCenter, yCenter, longAxis, shortAxis, color, true);
}

/**************************************************************************/
/*!
      Draws a HW accelerated curve on the display

      @args xCenter[in]   The 0-based x location of the ellipse's center
      @args yCenter[in]   The 0-based y location of the ellipse's center
      @args longAxis[in]  The size in pixels of the ellipse's long axis
      @args shortAxis[in] The size in pixels of the ellipse's short axis
      @args curvePart[in] The corner to draw, where in clock-wise motion:
                            0 = 180-270?                            1 = 270-0?                            2 = 0-90?                            3 = 90-180?      @args color[in]     The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void Adafruit_RA8875::drawCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color)
{
  curveHelper(xCenter, yCenter, longAxis, shortAxis, curvePart, color, false);
}

/**************************************************************************/
/*!
      Draws a HW accelerated filled curve on the display

      @args xCenter[in]   The 0-based x location of the ellipse's center
      @args yCenter[in]   The 0-based y location of the ellipse's center
      @args longAxis[in]  The size in pixels of the ellipse's long axis
      @args shortAxis[in] The size in pixels of the ellipse's short axis
      @args curvePart[in] The corner to draw, where in clock-wise motion:
                            0 = 180-270?                            1 = 270-0?                            2 = 0-90?                            3 = 90-180?      @args color[in]     The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void Adafruit_RA8875::fillCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color)
{
  curveHelper(xCenter, yCenter, longAxis, shortAxis, curvePart, color, true);
}
////////////Show the picture of the flash

/**************************************************************************/
/*!
      Helper function for higher level circle drawing code
*/
/**************************************************************************/
void Adafruit_RA8875::circleHelper(int16_t x0, int16_t y0, int16_t r, uint16_t color, bool filled)
{
  /* Set X */
  writeCommand(0x99);
  writeData(x0);
  writeCommand(0x9a);
  writeData(x0 >> 8);
  
  /* Set Y */
  writeCommand(0x9b);
  writeData(y0); 
  writeCommand(0x9c);	   
  writeData(y0 >> 8);
  
  /* Set Radius */
  writeCommand(0x9d);
  writeData(r);  
  
  /* Set Color */
  writeCommand(0x63);
  writeData((color & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((color & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((color & 0x001f));
  
  /* Draw! */
  writeCommand(RA8875_DCR);
  if (filled)
  {
    writeData(RA8875_DCR_CIRCLE_START | RA8875_DCR_FILL);
  }
  else
  {
    writeData(RA8875_DCR_CIRCLE_START | RA8875_DCR_NOFILL);
  }
  
  /* Wait for the command to finish */
  waitPoll(RA8875_DCR, RA8875_DCR_CIRCLE_STATUS);
}

/**************************************************************************/
/*!
      Helper function for higher level rectangle drawing code
*/
/**************************************************************************/
void Adafruit_RA8875::rectHelper(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool filled)
{
  /* Set X */
  writeCommand(0x91);
  writeData(x);
  writeCommand(0x92);
  writeData(x >> 8);
  
  /* Set Y */
  writeCommand(0x93);
  writeData(y); 
  writeCommand(0x94);	   
  writeData(y >> 8);
  
  /* Set X1 */
  writeCommand(0x95);
  writeData(w);
  writeCommand(0x96);
  writeData((w) >> 8);
  
  /* Set Y1 */
  writeCommand(0x97);
  writeData(h); 
  writeCommand(0x98);
  writeData((h) >> 8);

  /* Set Color */
  writeCommand(0x63);
  writeData((color & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((color & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((color & 0x001f));

  /* Draw! */
  writeCommand(RA8875_DCR);
  if (filled)
  {
    writeData(0xB0);
  }
  else
  {
    writeData(0x90);
  }
  
  /* Wait for the command to finish */
  waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

/**************************************************************************/
/*!
      Helper function for higher level triangle drawing code
*/
/**************************************************************************/
void Adafruit_RA8875::triangleHelper(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, bool filled)
{
  /* Set Point 0 */
  writeCommand(0x91);
  writeData(x0);
  writeCommand(0x92);
  writeData(x0 >> 8);
  writeCommand(0x93);
  writeData(y0); 
  writeCommand(0x94);
  writeData(y0 >> 8);

  /* Set Point 1 */
  writeCommand(0x95);
  writeData(x1);
  writeCommand(0x96);
  writeData(x1 >> 8);
  writeCommand(0x97);
  writeData(y1); 
  writeCommand(0x98);
  writeData(y1 >> 8);

  /* Set Point 2 */
  writeCommand(0xA9);
  writeData(x2);
  writeCommand(0xAA);
  writeData(x2 >> 8);
  writeCommand(0xAB);
  writeData(y2); 
  writeCommand(0xAC);
  writeData(y2 >> 8);
  
  /* Set Color */
  writeCommand(0x63);
  writeData((color & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((color & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((color & 0x001f));
  
  /* Draw! */
  writeCommand(RA8875_DCR);
  if (filled)
  {
    writeData(0xA1);
  }
  else
  {
    writeData(0x81);
  }
  
  /* Wait for the command to finish */
  waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

/**************************************************************************/
/*!
      Helper function for higher level ellipse drawing code
*/
/**************************************************************************/
void Adafruit_RA8875::ellipseHelper(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color, bool filled)
{
  /* Set Center Point */
  writeCommand(0xA5);
  writeData(xCenter);
  writeCommand(0xA6);
  writeData(xCenter >> 8);
  writeCommand(0xA7);
  writeData(yCenter); 
  writeCommand(0xA8);
  writeData(yCenter >> 8);

  /* Set Long and Short Axis */
  writeCommand(0xA1);
  writeData(longAxis);
  writeCommand(0xA2);
  writeData(longAxis >> 8);
  writeCommand(0xA3);
  writeData(shortAxis); 
  writeCommand(0xA4);
  writeData(shortAxis >> 8);
  
  /* Set Color */
  writeCommand(0x63);
  writeData((color & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((color & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((color & 0x001f));
  
  /* Draw! */
  writeCommand(0xA0);
  if (filled)
  {
    writeData(0xC0);
  }
  else
  {
    writeData(0x80);
  }
  
  /* Wait for the command to finish */
  waitPoll(RA8875_ELLIPSE, RA8875_ELLIPSE_STATUS);
}

/**************************************************************************/
/*!
      Helper function for higher level curve drawing code
*/
/**************************************************************************/
void Adafruit_RA8875::curveHelper(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color, bool filled)
{
  /* Set Center Point */
  writeCommand(0xA5);
  writeData(xCenter);
  writeCommand(0xA6);
  writeData(xCenter >> 8);
  writeCommand(0xA7);
  writeData(yCenter); 
  writeCommand(0xA8);
  writeData(yCenter >> 8);

  /* Set Long and Short Axis */
  writeCommand(0xA1);
  writeData(longAxis);
  writeCommand(0xA2);
  writeData(longAxis >> 8);
  writeCommand(0xA3);
  writeData(shortAxis); 
  writeCommand(0xA4);
  writeData(shortAxis >> 8);
  
  /* Set Color */
  writeCommand(0x63);
  writeData((color & 0xf800) >> 11);
  writeCommand(0x64);
  writeData((color & 0x07e0) >> 5);
  writeCommand(0x65);
  writeData((color & 0x001f));

  /* Draw! */
  writeCommand(0xA0);
  if (filled)
  {
    writeData(0xD0 | (curvePart & 0x03));
  }
  else
  {
    writeData(0x90 | (curvePart & 0x03));
  }
  
  /* Wait for the command to finish */
  waitPoll(RA8875_ELLIPSE, RA8875_ELLIPSE_STATUS);
}

/************************* Mid Level ***********************************/

/**************************************************************************/
/*!

*/
/**************************************************************************/
void Adafruit_RA8875::GPIOX(boolean on) {
  if (on)
    writeReg(RA8875_GPIOX, 1);
  else 
    writeReg(RA8875_GPIOX, 0);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void Adafruit_RA8875::PWM1out(uint8_t p) {
  writeReg(RA8875_P1DCR, p);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void Adafruit_RA8875::PWM2out(uint8_t p) {
  writeReg(RA8875_P2DCR, p);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void Adafruit_RA8875::PWM1config(boolean on, uint8_t clock) {
  if (on) {
    writeReg(RA8875_P1CR, RA8875_P1CR_ENABLE | (clock & 0xF));
  } else {
    writeReg(RA8875_P1CR, RA8875_P1CR_DISABLE | (clock & 0xF));
  }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void Adafruit_RA8875::PWM2config(boolean on, uint8_t clock) {
  if (on) {
    writeReg(RA8875_P2CR, RA8875_P2CR_ENABLE | (clock & 0xF));
  } else {
    writeReg(RA8875_P2CR, RA8875_P2CR_DISABLE | (clock & 0xF));
  }
}

/**************************************************************************/
/*!
      Enables or disables the on-chip touch screen controller
*/
/**************************************************************************/
void Adafruit_RA8875::touchEnable(boolean on) 
{
  if (on) 
  {
    /* Enable Touch Panel (Reg 0x70) */
    writeReg(RA8875_TPCR0, RA8875_TPCR0_ENABLE        | 
                           RA8875_TPCR0_WAIT_4096CLK  |
                           RA8875_TPCR0_WAKEDISABLE   | 
                           RA8875_TPCR0_ADCCLK_DIV4); // 10mhz max!
    /* Set Auto Mode      (Reg 0x71) */
    writeReg(RA8875_TPCR1, RA8875_TPCR1_AUTO    | 
                           // RA8875_TPCR1_VREFEXT | 
                           RA8875_TPCR1_DEBOUNCE);
    /* Enable TP INT */
    writeReg(RA8875_INTC1, readReg(RA8875_INTC1) | RA8875_INTC1_TP);
  } 
  else
  {
    /* Disable TP INT */
    writeReg(RA8875_INTC1, readReg(RA8875_INTC1) & ~RA8875_INTC1_TP);
    /* Disable Touch Panel (Reg 0x70) */
    writeReg(RA8875_TPCR0, RA8875_TPCR0_DISABLE);
  }
}

/**************************************************************************/
/*!
      Checks if a touch event has occured
      
      @returns  True is a touch event has occured (reading it via
                touchRead() will clear the interrupt in memory)
*/
/**************************************************************************/
boolean Adafruit_RA8875::touched(void) 
{
  if (readReg(RA8875_INTC2) & RA8875_INTC2_TP) return true;
  return false;
}

/**************************************************************************/
/*!
      Reads the last touch event
      
      @args x[out]  Pointer to the uint16_t field to assign the raw X value
      @args y[out]  Pointer to the uint16_t field to assign the raw Y value
      
      @note Calling this function will clear the touch panel interrupt on
            the RA8875, resetting the flag used by the 'touched' function
*/
/**************************************************************************/
boolean Adafruit_RA8875::touchRead(uint16_t *x, uint16_t *y) 
{
  uint16_t tx, ty;
  uint8_t temp;
  
  tx = readReg(RA8875_TPXH);
  ty = readReg(RA8875_TPYH);
  temp = readReg(RA8875_TPXYL);
  tx <<= 2;
  ty <<= 2;
  tx |= temp & 0x03;        // get the bottom x bits
  ty |= (temp >> 2) & 0x03; // get the bottom y bits

  *x = tx;
  *y = ty;

  /* Clear TP INT Status */
  writeReg(RA8875_INTC2, RA8875_INTC2_TP);

  return true;
}

/**************************************************************************/
/*!
      Turns the display on or off
*/
/**************************************************************************/
void Adafruit_RA8875::displayOn(boolean on) 
{
 if (on) 
   writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPON);
 else
   writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPOFF);
}

/**************************************************************************/
/*!
    Puts the display in sleep mode, or disables sleep mode if enabled
*/
/**************************************************************************/
void Adafruit_RA8875::sleep(boolean sleep) 
{
 if (sleep) 
   writeReg(RA8875_PWRR, RA8875_PWRR_DISPOFF | RA8875_PWRR_SLEEP);
 else
   writeReg(RA8875_PWRR, RA8875_PWRR_DISPOFF);
}

/************************* Low Level ***********************************/

/**************************************************************************/
/*!

*/
/**************************************************************************/
void  Adafruit_RA8875::writeReg(uint8_t reg, uint8_t val) 
{
  writeCommand(reg);
  writeData(val);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
uint8_t  Adafruit_RA8875::readReg(uint8_t reg) 
{
  writeCommand(reg);
  return readData();
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void  Adafruit_RA8875::writeData(uint8_t d) 
{
  digitalWrite(_cs, LOW);
  SPI.transfer(RA8875_DATAWRITE);
  SPI.transfer(d);
  digitalWrite(_cs, HIGH);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
uint8_t  Adafruit_RA8875::readData(void) 
{
  digitalWrite(_cs, LOW);
  SPI.transfer(RA8875_DATAREAD);
  uint8_t x = SPI.transfer(0x0);
  digitalWrite(_cs, HIGH);
  return x;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void  Adafruit_RA8875::writeCommand(uint8_t d) 
{
  digitalWrite(_cs, LOW);
  SPI.transfer(RA8875_CMDWRITE);
  SPI.transfer(d);
  digitalWrite(_cs, HIGH);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
uint8_t  Adafruit_RA8875::readStatus(void) 
{
  digitalWrite(_cs, LOW);
  SPI.transfer(RA8875_CMDREAD);
  uint8_t x = SPI.transfer(0x0);
  digitalWrite(_cs, HIGH);
  return x;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/

void Adafruit_RA8875::Active_Window(uint XL,uint XR ,uint YT ,uint YB)
{
	uchar temp;
    //setting active window X
	temp=XL;   
    writeCommand(0x30);//HSAW0
	writeData(temp);
	temp=XL>>8;   
    writeCommand(0x31);//HSAW1	   
	writeData(temp);

	temp=XR;   
    writeCommand(0x34);//HEAW0
	writeData(temp);
	temp=XR>>8;   
    writeCommand(0x35);//HEAW1	   
	writeData(temp);

    //setting active window Y
	temp=YT;   
    writeCommand(0x32);//VSAW0
	writeData(temp);
	temp=YT>>8;   
    writeCommand(0x33);//VSAW1	   
	writeData(temp);

	temp=YB;   
    writeCommand(0x36);//VEAW0
	writeData(temp);
	temp=YB>>8;   
    writeCommand(0x37);//VEAW1	   
	writeData(temp);
}
///////////////check busy

void Adafruit_RA8875::Chk_Busy(void)
{
	uchar temp; 	
	do
	{
	temp=readStatus();
	}while((temp&0x80)==0x80);		   
}
///////////////check bte busy
void Adafruit_RA8875::Chk_BTE_Busy(void)
{
	uchar temp; 	
	do
	{
	temp=readStatus();
	}while((temp&0x40)==0x40);		   
}
///////////////check dma busy
void Adafruit_RA8875::Chk_DMA_Busy(void)
{
	uchar temp; 	
	do
	{
	writeCommand(0xBF);
	temp =readStatus();
	}while(temp==0x01);   
}
void Adafruit_RA8875::Scroll_Window(uint XL,uint XR ,uint YT ,uint YB)
{
	uchar temp;    
	temp=XL;   
    writeCommand(0x38);//HSSW0
	writeData(temp);
	temp=XL>>8;   
    writeCommand(0x39);//HSSW1	   
	writeData(temp);

	temp=XR;   
    writeCommand(0x3c);//HESW0
	writeData(temp);
	temp=XR>>8;   
    writeCommand(0x3d);//HESW1	   
	writeData(temp);   
    
	temp=YT;   
    writeCommand(0x3a);//VSSW0
	writeData(temp);
	temp=YT>>8;   
    writeCommand(0x3b);//VSSW1	   
	writeData(temp);

	temp=YB;   
    writeCommand(0x3e);//VESW0
	writeData(temp);
	temp=YB>>8;   
    writeCommand(0x3f);//VESW1	   
	writeData(temp);
}  
void Adafruit_RA8875::DMA_Start_enable(void)
{  uint8_t temp;
  writeCommand(0xBF);
  temp =  readReg(0xBF);
  temp |= 0x01;
  writeData(temp);
}

void Adafruit_RA8875::XY_Coordinate(uint16_t X,uint16_t Y)
{
    writeCommand(0x46);
	writeData(X);  
    writeCommand(0x47);	   
	writeData(X>>8);
 
    writeCommand(0x48);
	writeData(Y);  
    writeCommand(0x49);	   
	writeData(Y>>8);
}

///////////////Window scroll offset Settings
void Adafruit_RA8875::Scroll(uint X,uint Y)
{
	uchar temp;
    
	temp=X;   
    writeCommand(0x24);//HOFS0
	writeData(temp);
	temp=X>>8;   
    writeCommand(0x25);//HOFS1	   
	writeData(temp);

	temp=Y;   
    writeCommand(0x26);//VOFS0
	writeData(temp);
	temp=Y>>8;   
    writeCommand(0x27);//VOFS1	   
	writeData(temp); 
}	   	  

///////////////The FLASH reading area   setting
void Adafruit_RA8875::DMA_block_move(uint16_t X,uint16_t Y,uint16_t BWR,uint16_t BHR,uint16_t SPWR,uint64_t start_address)
{  
   Write_Dir(0X06,0X00);//FLASH frequency setting
   Write_Dir(0X05,0X87);//FLASH setting 
   writeCommand(0xBF);
   writeData(0x02);

   XY_Coordinate(X,Y);
    Active_Window(X,X+BWR-1,Y,Y+BHR);  
   DMA_block_mode_size_setting(BWR,BHR,SPWR);
   DMA_Start_address_setting(start_address);      
   //DMA_Start_enable();
    writeCommand(0xBF);
   writeData(0x03);
   Chk_DMA_Busy();
 //  	 Serial.println(readReg(0x04),BIN);
  //Serial.println(readReg(0x05),BIN);
  //Serial.println(readReg(0x06),BIN);

 }
void Adafruit_RA8875::DMA_block_mode_size_setting(uint BWR,uint BHR,uint SPWR)
{
  	writeCommand(0xB4);
  	writeData(BWR);
  	writeCommand(0xB5);
  	writeData(BWR>>8);

  	writeCommand(0xB6);
  	writeData(BHR);
  	writeCommand(0xB7);
  	writeData(BHR>>8);

  	writeCommand(0xB8);
  	writeData(SPWR);
  	writeCommand(0xB9);
  	writeData(SPWR>>8);  
}
void Adafruit_RA8875::writeTo(uint8_t d){
	 Serial.println("Writetos");
	  Serial.println(readReg(0x20),BIN);
	 Serial.println(readReg(0x52),BIN);
  Serial.println(readReg(0x53),BIN);
 Serial.println("Writetoe");
	uint8_t temp = readReg(RA8875_MWCR1);
	switch(d){
	
		case 1:
			temp &= ~((1<<3) | (1<<2));// Clear bits 3 and 2
			temp &= ~(1 << 0); //clear bit 0
			currentLayer = 0;
		break;
		case 2:
			temp &= ~((1<<3) | (1<<2));// Clear bits 3 and 2
			temp |= (1 << 0); //bit set 0
			currentLayer = 1;
		break;
		
		case 3: 
			temp &= ~(1 << 3); //clear bit 3
			temp |= (1 << 2); //bit set 2
			if (bitRead(_FNCR0Reg,7)){//REG[0x21] bit7 must be 0
				_FNCR0Reg &= ~(1 << 7); //clear bit 7
				writeReg(RA8875_FNCR0,_FNCR0Reg);  
			}
		break;
		case 4:
			temp |= (1 << 3); //bit set 3
			temp |= (1 << 2); //bit set 2
		break;
		case 5:
			temp |= (1 << 3); //bit set 3
			temp &= ~(1 << 2); //clear bit 2
		break;
		default:
		break;
	}
	writeReg(RA8875_MWCR1,temp);  
}
////////////Show the picture of the flash
void  Adafruit_RA8875::setflash(void) {
	writeCommand(0x06);	writeData(0);
  	writeReg(0x05, readReg(0x05) | cSetD7);
	uint8_t temp;
	writeCommand(0x05);//LTPR0
	temp = readReg(0x05);
	temp&=0xe7;
	temp|=cSetD3;
	writeData(temp);  
	 	writeReg(0x05, readReg(0x05) | cSetD2);
		writeCommand(0xBF);	writeData(0x02);
	
		
}
void Adafruit_RA8875::layerEffect(enum RA8875boolean efx){
	uint8_t	reg = 0b00000000;
	//reg &= ~(0x07);//clear bit 2,1,0
	switch(efx){//                       bit 2,1,0 of LTPR0
		case LAYER1: //only layer 1 visible  [000]
			//do nothing
		break;
		case LAYER2: //only layer 2 visible  [001]
			reg |= (1 << 0);
		break;
		case TRANSPARENT: //transparent mode [011]
			reg |= (1 << 0); reg |= (1 << 1);
		break;
		case LIGHTEN: //lighten-overlay mode [010]
			reg |= (1 << 1);
		break;
		case OR: //boolean OR mode           [100]
			reg |= (1 << 2);
		break;
		case AND: //boolean AND mode         [101]
			reg |= (1 << 0); reg |= (1 << 2);
		break;
		case FLOATING: //floating windows    [110]
			reg |= (1 << 1); reg |= (1 << 2);
		break;
		default:
			//do nothing
		break;
	}
	 writeReg(RA8875_LTPR0,reg);
}
void Adafruit_RA8875::dispic(uchar picnum)
{  
   uchar picnumtemp;
   Write_Dir(0X06,0X00);//FLASH frequency setting
   Write_Dir(0X05,0X87);//FLASH setting 

	picnumtemp=picnum;

   Write_Dir(0XBF,0X02);//FLASH setting
   Active_Window(0,479,0,271); 
   MemoryWrite_Position(0,0);//Memory write position
   DMA_Start_address_setting(2084300);//DMA Start address setting
   DMA_block_mode_size_setting(480,272,480);   
   Write_Dir(0XBF,0X03);//FLASH setting
		 Chk_DMA_Busy();
		 delay(50);
} 
void Adafruit_RA8875::dispicown(uint16_t x,uint16_t y, uint16_t w,uint16_t h,uint64_t start)
{  
 waitPoll(0xBF,0x01);

 DMA_block_move(x,y,w,h,w,start);
 
  //Chk_DMA_Busy();

} 
void Adafruit_RA8875::Transparent_Mode(void)
{	//Write_Dir(0X52,0X22);//FLASH setting  
//Write_Dir(0X52,0X60);

byte b1 = B00000011;
Write_Dir(0X52,b1);
 b1 = B00000000;
	Write_Dir(0X53,b1);
	b1=10000001;
		Write_Dir(0X20,b1);//FLASH setting
	//Write_Dir(0X53,0X00);//FLASH setting
	//uint8_t temp = readReg(0x52);
	//temp&=0xf8;
	//temp|=0x03;
	//writeData(temp);  
	Write_Dir(0X67,0X00);
	Write_Dir(0X68,0X00);
	Write_Dir(0X69,0X00);
//	 Serial.println(readReg(0x20),BIN);
//  Serial.println(readReg(0x53),BIN);

}
void Adafruit_RA8875::layer2_1_transparency(uint8_t setx)
{  
   writeCommand(0x53);//LTPR1
   writeData(setx); 
}
void Adafruit_RA8875::DMA_Continuous_mode_size_setting(uint16_t set_size)
{ 
  writeCommand(0xB4);
  writeData(set_size);

  writeCommand(0xB6);
  writeData(set_size>>8);
 
  writeCommand(0xB8);
  writeData(set_size>>16);

}
void Adafruit_RA8875::DMA_Continuous_mode(void)
{ 
  writeCommand(0xBF);
  writeData(0x00);
}

void Adafruit_RA8875::MemoryWrite_Position(uint X,uint Y)
{
	uchar temp;

	temp=X;   
    writeCommand(0x46);
	writeData(temp);
	temp=X>>8;   
    writeCommand(0x47);	   
	writeData(temp);

	temp=Y;   
    writeCommand(0x48);
	writeData(temp);
	temp=Y>>8;   
    writeCommand(0x49);	   
	writeData(temp);
}
void Adafruit_RA8875::Delay1ms(uint i)
{uchar j;
	while(i--)
  	for(j=0;j<125;j++);
}


void Adafruit_RA8875::Delay10ms(uint i)
{	while(i--)
	Delay1ms(10);
}

void Adafruit_RA8875::Delay100ms(uint i)
{	while(i--)
	Delay1ms(100);
}

void Adafruit_RA8875::NextStep(void)
{ 
 	while(next)
		{
			Delay100ms(1);
		}
	while(!next);
	Delay100ms(12);
	while(!next);
}
void Adafruit_RA8875::CutPictrue(uchar picnum,uint x1,uint y1,uint x2,uint y2,unsigned long x,unsigned long y)
{unsigned long cutaddress;uchar picnumtemp;
    Write_Dir(0X06,0X00);//FLASH frequency setting   
 
 Write_Dir(0X05,0X87);//FLASH setting 
	picnumtemp=picnum;
   
   	Write_Dir(0XBF,0X02);//FLASH setting
   	Active_Window(x1,x2,y1,y2);		
   	MemoryWrite_Position(x1,y1);//Memory write position
   	cutaddress=(picnumtemp-1)*5320+y*70+x*2;
   	DMA_Start_address_setting(cutaddress);
   	DMA_block_mode_size_setting(x2-x1+1,y2-y1+1,34);
   	Write_Dir(0XBF,0X03);//FLASH setting
	Chk_DMA_Busy();
}

/*
void Adafruit_RA8875::CutPictrue(uchar picnum,uint x1,uint y1,uint x2,uint y2,unsigned long x,unsigned long y)
{unsigned long cutaddress;uchar picnumtemp;
    Write_Dir(0X06,0X00);//FLASH frequency setting   
   //	Write_Dir(0X05,0Xac);//FLASH setting
 Write_Dir(0X05,0X87);//FLASH setting 
	picnumtemp=picnum;
   
   	Write_Dir(0XBF,0X02);//FLASH setting
   	Active_Window(x1,x2,y1,y2);		
   	MemoryWrite_Position(x1,y1);//Memory write position
   	cutaddress=(picnumtemp-1)*261120+y*960+x*2;
   	DMA_Start_address_setting(cutaddress);
   	DMA_block_mode_size_setting(x2-x1+1,y2-y1+1,480);
   	Write_Dir(0XBF,0X03);//FLASH setting
	Chk_DMA_Busy();
}
*/
void  Adafruit_RA8875::DMA_Start_address_setting(ulong set_address)
{ 
  	writeCommand(0xB0);
  	writeData(set_address);

  	writeCommand(0xB1);
  	writeData(set_address>>8);

	writeCommand(0xB2);
  	writeData(set_address>>16);

  	writeCommand(0xB3);
  	writeData(set_address>>24);
}
void Adafruit_RA8875::BTE_Size(uint width,uint height)
{
    uchar temp;
	temp=width;   
    writeCommand(0x5c);//BET area width literacy
	writeData(temp);
	temp=width>>8;   
    writeCommand(0x5d);//BET area width literacy	   
	writeData(temp);

	temp=height;   
    writeCommand(0x5e);//BET area height literacy
	writeData(temp);
	temp=height>>8;   
    writeCommand(0x5f);//BET area height literacy	   
	writeData(temp);
}		

////////////////////BTE starting position
void Adafruit_RA8875::BTE_Source(uint SX,uint DX ,uint SY ,uint DY)
{
	uchar temp,temp1;
    
	temp=SX;   
    writeCommand(0x54);//BTE horizontal position of read/write data
	writeData(temp);
	temp=SX>>8;   
    writeCommand(0x55);//BTE horizontal position of read/write data   
	writeData(temp);

	temp=DX;
    writeCommand(0x58);//BET written to the target horizontal position
	writeData(temp);
	temp=DX>>8;   
    writeCommand(0x59);//BET written to the target horizontal position	   
	writeData(temp); 
    
	temp=SY;   
    writeCommand(0x56);//BTE vertical position of read/write data
	writeData(temp);
	temp=SY>>8;   
    writeCommand(0x57);
	temp1 = readData();
	temp1 &= 0x80;
    temp=temp|temp1; 
	writeCommand(0x57);//BTE vertical position of read/write data  
	writeData(temp);

	temp=DY;   
    writeCommand(0x5a);//BET written to the target  vertical  position
	writeData(temp);
	temp=DY>>8;   
    writeCommand(0x5b);
	temp1 =readData();
	temp1 &= 0x80;
	temp=temp|temp1;	
	writeCommand(0x5b);//BET written to the target  vertical  position 
	writeData(temp);
}				

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+								BTE STUFF											 +
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/

/**************************************************************************/
/* 
		Block Transfer Move
		Can move a rectangular block from any area of memory (eg. layer 1) to any other (eg layer 2)
		Can move with transparency - note THE TRANSPARENT COLOUR IS THE TEXT FOREGROUND COLOUR
		ReverseDir is for moving overlapping areas - may need to use reverse to prevent it double-copying the overlapping area (this option not available with transparency or monochrome)
		ROP is Raster Operation. Usually use RA8875_ROP_SOURCE but a few others are defined
		Defaults to current layer if not given or layer is zero.
		Monochrome uses the colour-expansion mode: the input is a bit map which is then converted to the current foreground and background colours, transparent background is optional
		Monochrome data is assumed to be linear, originally written to the screen memory in 16-bit chunks with drawPixels().
		Monochrome mode uses the ROP to define the offset of the first image bit within the first byte. This also depends on the width of the block you are trying to display.
		Monochrome skips 16-bit words in the input pattern - see the example for more explanation and a trick to interleave 2 characters in the space of one.

		This function returns immediately but the actual transfer can take some time
		Caller should check the busy status before issuing any more RS8875 commands.

		Basic usage:
		BTE_Move(SourceX, SourceY, Width, Height, DestX, DestY) = copy something visible on the current layer
		BTE_Move(SourceX, SourceY, Width, Height, DestX, DestY, 2) = copy something from layer 2 to the current layer
		BTE_Move(SourceX, SourceY, Width, Height, DestX, DestY, 2, 1, true) = copy from layer 2 to layer 1, with the transparency option
		BTE_Move(SourceX, SourceY, Width, Height, DestX, DestY, 0, 0, true, RA8875_BTEROP_ADD) = copy on the current layer, using transparency and the ADD/brighter operation 
		BTE_Move(SourceX, SourceY, Width, Height, DestX, DestY, 0, 0, false, RA8875_BTEROP_SOURCE, false, true) = copy on the current layer using the reverse direction option for overlapping areas
*/

void  Adafruit_RA8875::BTE_move(int16_t SourceX, int16_t SourceY, int16_t Width, int16_t Height, int16_t DestX, int16_t DestY, uint8_t SourceLayer, uint8_t DestLayer,bool Transparent, uint8_t ROP, bool Monochrome, bool ReverseDir)
{

	//Monochrome=false;
	if (_currentMode) changeMode(false);//we are in text mode?
	if (SourceLayer == 0) SourceLayer = _currentLayer;	
	if (DestLayer == 0) DestLayer = _currentLayer;
	if (SourceLayer == 2) SourceY |= 0x8000; //set the high bit of the vertical coordinate to indicate layer 2
	if (DestLayer == 2) DestY |= 0x8000; //set the high bit of the vertical coordinate to indicate layer 2
	ROP &= 0xF0; //Ensure the lower bits of ROP are zero
	if (Transparent) {
		if (Monochrome) {
			ROP |= 0x0A; //colour-expand transparent
		} else {
			ROP |= 0x05; //set the transparency option 
		}
	} else {
		if (Monochrome) {
			ROP |= 0x0B; //colour-expand normal
		} else {
			if (ReverseDir) {
				ROP |= 0x03; //set the reverse option
			} else {
				ROP |= 0x02; //standard block-move operation
			}
		}
	}

	waitBusy(0x40); //Check that another BTE operation is not still in progress
	BTE_moveFrom(SourceX,SourceY);
	BTE_size(Width,Height);
	BTE_moveTo(DestX,DestY);
	BTE_ropcode(ROP);
	//Execute BTE! (This selects linear addressing mode for the monochrome source data)
	if (Monochrome) writeReg(RA8875_BECR0, 0xC0); else writeReg(RA8875_BECR0, 0x80);
	waitBusy(0x40);
	//we are supposed to wait for the thing to become unbusy
	//caller can call waitBusy(0x40) to check the BTE busy status (except it's private)
}

/**************************************************************************/
/*! TESTING

*/
/**************************************************************************/
void Adafruit_RA8875::BTE_size(int16_t w, int16_t h)
{
	//0.69b21 -have to check this, not verified
	if (_portrait) swapvals(w,h);
    writeReg(RA8875_BEWR0,w & 0xFF);//BET area width literacy  
    writeReg(RA8875_BEWR1,w >> 8);//BET area width literacy	   
    writeReg(RA8875_BEHR0,h & 0xFF);//BET area height literacy
    writeReg(RA8875_BEHR1,h >> 8);//BET area height literacy	   
}	

/**************************************************************************/
/*!

*/
/**************************************************************************/

void Adafruit_RA8875::BTE_moveFrom(int16_t SX,int16_t SY)
{
	//if (_portrait) swapvals(SX,SY);
	writeReg(RA8875_HSBE0,SX & 0xFF);
	writeReg(RA8875_HSBE1,SX >> 8);
	writeReg(RA8875_VSBE0,SY & 0xFF);
	writeReg(RA8875_VSBE1,SY >> 8);
}	

/**************************************************************************/
/*!

*/
/**************************************************************************/

void Adafruit_RA8875::BTE_moveTo(int16_t DX,int16_t DY)
{
	if (_portrait) swapvals(DX,DY);
	writeReg(RA8875_HDBE0,DX & 0xFF);
	writeReg(RA8875_HDBE1,DX >> 8);
	writeReg(RA8875_VDBE0,DY & 0xFF);
	writeReg(RA8875_VDBE1,DY >> 8);
}	

/**************************************************************************/
/*! TESTING
	Use a ROP code EFX
*/
/**************************************************************************/
void Adafruit_RA8875::BTE_ropcode(unsigned char setx)
{
    writeReg(RA8875_BECR1,setx);//BECR1	   
}

/**************************************************************************/
/*! TESTING
	Enable BTE transfer
*/
/**************************************************************************/
void Adafruit_RA8875::BTE_enable(bool on) 
{	
	uint8_t temp = readReg(RA8875_BECR0);
	on == true ? temp &= ~(1 << 7) : temp |= (1 << 7);
	writeData(temp);
	//writeReg(RA8875_BECR0,temp);  
	waitBusy(0x40);
}


/**************************************************************************/
/*! TESTING
	Select BTE mode (CONT (continuous) or RECT)
*/
/**************************************************************************/
void Adafruit_RA8875::BTE_dataMode(enum RA8875btedatam m) 
{	
	uint8_t temp = readReg(RA8875_BECR0);
	m == CONT ? temp &= ~(1 << 6) : temp |= (1 << 6);
	writeData(temp);
	//writeReg(RA8875_BECR0,temp);  
}

/**************************************************************************/
/*! TESTING
	Select the BTE SOURCE or DEST layer (1 or 2)
*/
/**************************************************************************/

void Adafruit_RA8875::BTE_layer(enum RA8875btelayer sd,uint8_t l)
{
	uint8_t temp;
	/*
	if (sd != SOURCE){//destination
		temp = readReg(RA8875_VDBE1);
	} else {//source
		temp = readReg(RA8875_VSBE1);
	}
	*/
	sd == SOURCE ? temp = readReg(RA8875_VSBE1) : temp = readReg(RA8875_VDBE1);
	//l == 1 ? bitClear(temp,7) : bitSet(temp,7);
	l == 1 ? temp &= ~(1 << 7) : temp |= (1 << 7);
	writeData(temp);
	//writeReg(RA8875_VSBE1,temp);  
}
void Adafruit_RA8875::changeMode(bool m) 
{
	if (m == _currentMode) return;
	writeCommand(RA8875_MWCR0);
	//if (m != 0){//text
	if (m){//text
		_MWCR0Reg |= (1 << 7);
		_currentMode = true;
	} else {//graph
		_MWCR0Reg &= ~(1 << 7);
		_currentMode = false;
	}
	writeData(_MWCR0Reg);
}
void Adafruit_RA8875::waitBusy(uint8_t res) 
{
	uint8_t temp; 	
	unsigned long start = millis();//M.Sandercock
	do {
	if (res == 0x01) writeCommand(RA8875_DMACR);//dma
	temp = readStatus();
	if ((millis() - start) > 10) return;
	} while ((temp & res) == res);
}
// }
// void Adafruit_RA8875::BTE_size(int16_t w, int16_t h)
// {
	//0.69b21 -have to check this, not verified
	// if (_portrait) swapvals(w,h);
    // writeReg(RA8875_BEWR0,w & 0xFF);//BET area width literacy  
    // writeReg(RA8875_BEWR1,w >> 8);//BET area width literacy	   
    // writeReg(RA8875_BEHR0,h & 0xFF);//BET area height literacy
    // writeReg(RA8875_BEHR1,h >> 8);//BET area height literacy	   
// }	