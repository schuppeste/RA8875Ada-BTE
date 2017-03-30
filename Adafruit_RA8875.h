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

#if ARDUINO >= 100
 #include "Arduino.h"
 #include "Print.h"
#else
 #include "WProgram.h"
#endif

#ifdef __AVR__
  #include <avr/pgmspace.h>
#endif

#include <Adafruit_GFX.h>

#ifndef _ADAFRUIT_RA8875_H
#define _ADAFRUIT_RA8875_H
#define uchar      unsigned char
#define uint       unsigned int
#define ulong      unsigned long

// Sizes!
enum RA8875sizes { RA8875_480x272, RA8875_800x480 };
          	  #define	cSetD3		     0x08
#define	cSetD2		     0x04
// Touch screen cal structs
typedef struct Point 
{
  int32_t x;
  int32_t y;
} tsPoint_t;

typedef struct //Matrix
{
  int32_t An,
          Bn,
          Cn,
          Dn,
          En,
          Fn,
          Divider ;
} tsMatrix_t;
enum RA8875pattern{ P8X8, P16X16 };
enum RA8875btedatam{ CONT, RECT };
enum RA8875btelayer{ SOURCE, DEST };
enum RA8875boolean { LAYER1, LAYER2, TRANSPARENT, LIGHTEN, OR, AND, FLOATING };//for LTPR0
#include "_utility/RA8875Registers.h"
class Adafruit_RA8875 : public Adafruit_GFX {
 public:
 	uint8_t					maxLayers;
	uint8_t		_FNCR0Reg; 
#define RA8875_FNCR0				  0x21//Font Control Register 0
#define RA8875_MWCR1  					0x41//Memory Write Control Register 1
#define RA8875_LTPR0            	  0x52

//BTE Raster OPerations - there's 16 possible operations but these are the main ones likely to be useful
#define RA8875_BTEROP_SOURCE	0xC0	//Overwrite dest with source (no mixing) *****THIS IS THE DEFAULT OPTION****
#define RA8875_BTEROP_BLACK		0xo0	//all black
#define RA8875_BTEROP_WHITE		0xf0	//all white
#define RA8875_BTEROP_DEST		0xA0    //destination unchanged
#define RA8875_BTEROP_ADD		0xE0    //ADD (brighter)
#define RA8875_BTEROP_SUBTRACT	0x20	//SUBTRACT (darker)
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                            Color Registers
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define RA8875_BGCR0				  0x60//Background Color Register 0 (R)
#define RA8875_BGCR1				  0x61//Background Color Register 1 (G)
#define RA8875_BGCR2				  0x62//Background Color Register 2 (B)
#define RA8875_FGCR0				  0x63//Foreground Color Register 0 (R)
#define RA8875_FGCR1				  0x64//Foreground Color Register 1 (G)
#define RA8875_FGCR2				  0x65//Foreground Color Register 2 (B)
#define RA8875_BGTR0				  0x67//Background Color Register for Transparent 0 (R)
#define RA8875_BGTR1				  0x68//Background Color Register for Transparent 1 (G)
#define RA8875_BGTR2				  0x69//Background Color Register for Transparent 2 (B)
#if !defined(swapvals)
	#define swapvals(a, b) { typeof(a) t = a; a = b; b = t; }
#endif
	volatile uint8_t		_MWCR0Reg; //keep track of the register 		  [0x40]
		void 	changeMode(bool m);
		bool					_portrait;
	uint8_t					currentLayer;
		int16_t					_cursorX, _cursorY;//try to internally track text cursor...
			//scroll vars ----------------------------
	int16_t					_scrollXL,_scrollXR,_scrollYT,_scrollYB;
		
		uint8_t					_brightness;
	uint8_t					_rotation;
	uint8_t					_initIndex;

	bool					_sleep;


	uint8_t					_maxLayers;
	bool					_useMultiLayers;
	uint8_t					_currentLayer;

	bool 					_hasLayerLimits;//helper
		bool					_inited;//true when init has been ended
	uint8_t					_fontSpacing;
	uint8_t					_fontInterline;

	int16_t 		 		WIDTH, HEIGHT;//absolute
	int16_t					_activeWindowXL;
	int16_t					_activeWindowXR;
	int16_t					_activeWindowYT;
	int16_t					_activeWindowYB;
	uint8_t					_errorCode;
  Adafruit_RA8875(uint8_t cs, uint8_t rst);
  enum RA8875writes { L1, L2, CGRAM, PATTERN, CURSOR };
  boolean begin(enum RA8875sizes s);
  void    softReset(void);
  void    displayOn(boolean on);
  void    sleep(boolean sleep);
  	void 		waitBusy(uint8_t res=0x80);//0x80, 0x40(BTE busy), 0x01(DMA busy)
	void 		BTE_size(int16_t w, int16_t h);
  	//boolean 	useLayers(boolean on);
	void		writeTo(enum RA8875writes d);//TESTING
	//void 		layerEffect(enum RA8875boolean efx);
	void 		layerTransparency(uint8_t layer1,uint8_t layer2);
void  layerEffect(enum RA8875boolean efx);
void	Text_Foreground_Color1(uint b_color);
void	Text_Background_Color1(uint b_color);
void Write_Dir(uchar Cmd,uchar Data);
void String(const char *str);
//void setTransparentColor(uint16_t color);
void Scroll_Window(uint XL,uint XR ,uint YT ,uint YB);
void Scroll(uint X,uint Y);
void DMA_block_mode_size_setting(uint BWR,uint BHR,uint SPWR);
void DMA_Start_address_setting(ulong set_address);
void DMA_block_move(uint16_t X,uint16_t Y,uint16_t BWR,uint16_t BHR,uint16_t SPWR,uint64_t start_address);
  void dispicown(uint16_t x,uint16_t y, uint16_t w,uint16_t h,uint64_t start);
  void DMA_Start_enable(void);
void writeTo(uint8_t d);
void XY_Coordinate(uint16_t X,uint16_t Y);

  /* Text functions */

  void    textMode(void);
  void    textSetCursor(uint16_t x, uint16_t y);
  void    FontWrite_Position(uint16_t x, uint16_t y);
  void    textColor(uint16_t foreColor, uint16_t bgColor);
  void    textTransparent(uint16_t foreColor);
  void    textEnlarge(uint8_t scale);
  void    textWrite(const char* buffer, uint16_t len=0);
  void		Active_Window(uint XL,uint XR ,uint YT ,uint YB);
  void Chk_Busy(void);
  void Chk_BTE_Busy(void);
  void Chk_DMA_Busy(void);
  void MemoryWrite_Position(uint X,uint Y);
  void Delay1ms(uint i);
  void Delay10ms(uint i);
  void Delay100ms(uint i);
  void NextStep(void);
  void CutPictrue(uchar picnum,uint x1,uint y1,uint x2,uint y2,unsigned long x,unsigned long y);
  //void	 	BTE_fromTo(int16_t SX,int16_t DX ,int16_t SY ,int16_t DY);
	void	 	BTE_moveFrom(int16_t SX,int16_t SY);
	void	 	BTE_moveTo(int16_t DX,int16_t DY);
	//void 		BTE_source_destination(uint16_t SX,uint16_t DX ,uint16_t SY ,uint16_t DY);
	void		BTE_ropcode(unsigned char setx);//
	void 		BTE_enable(bool on);//
	void 		BTE_dataMode(enum RA8875btedatam m);//CONT,RECT
	void 		BTE_layer(enum RA8875btelayer sd,uint8_t l);//SOURCE,DEST - 1 or 2
	void		BTE_move(int16_t SourceX, int16_t SourceY, int16_t Width, int16_t Height, int16_t DestX, int16_t DestY, uint8_t SourceLayer=0, uint8_t DestLayer=0, bool Transparent = false, uint8_t ROP=RA8875_BTEROP_SOURCE, bool Monochrome=false, bool ReverseDir = false);

//  void DMA_block_mode_size_setting(uint BWR,uint BHR,uint SPWR);
  void dispic(uchar picnum);

  void DMA_Continuous_mode(void);
  void DMA_Continuous_mode_size_setting(uint16_t set_size);
  /* Graphics functions */
  void Transparent_Mode(void);
  void layer2_1_transparency(uint8_t setx);
  void    graphicsMode(void);
  void    setXY(uint16_t x, uint16_t y);
  void    pushPixels(uint32_t num, uint16_t p);
  void    fillRect(void);
  /* Adafruit_GFX functions */
  void    drawPixel(int16_t x, int16_t y, uint16_t color);
  void    drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  void    drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
  
  /* HW accelerated wrapper functions (override Adafruit_GFX prototypes) */
 
  void    fillScreen(uint16_t color);
  void BTE_Source(uint SX,uint DX ,uint SY ,uint DY);
  void BTE_Size(uint width,uint height);
  void    drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
  void    drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void    fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void    drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
  void    fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
  void    drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
  void    fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
  void    drawEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color);
  void    fillEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color);
  void    drawCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color);
  void    fillCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color);
  void setlash(void);
  /* Backlight */
  void    GPIOX(boolean on);
  void    PWM1config(boolean on, uint8_t clock);
  void    PWM2config(boolean on, uint8_t clock);
  void    PWM1out(uint8_t p);
  void    PWM2out(uint8_t p);
void setflash(void);
  /* Touch screen */
  void    touchEnable(boolean on);
  boolean touched(void);
  boolean touchRead(uint16_t *x, uint16_t *y);

  /* Low level access */
  void    writeReg(uint8_t reg, uint8_t val);
  uint8_t readReg(uint8_t reg);
  void    writeData(uint8_t d);
  uint8_t readData(void);
  void    writeCommand(uint8_t d);
  uint8_t readStatus(void);
  boolean waitPoll(uint8_t r, uint8_t f);
  boolean waitPollvar(uint8_t r, uint8_t f);
  uint16_t width(void);
  uint16_t height(void);
#define	cSetD7		     0x80

  /* Play nice with Arduino's Print class */
  virtual size_t write(uint8_t b) {
    textWrite((const char *)&b, 1);
    return 1;
  }
  virtual size_t write(const uint8_t *buffer, size_t size) {
    textWrite((const char *)buffer, size);
    return size;
  }

 private:
  void PLLinit(void);
  void initialize(void);
  
  /* GFX Helper Functions */

  void circleHelper(int16_t x0, int16_t y0, int16_t r, uint16_t color, bool filled);
  void rectHelper  (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool filled);
  void triangleHelper(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, bool filled);
  void ellipseHelper(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color, bool filled);
  void curveHelper(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color, bool filled);

  uint8_t _cs, _rst;
  uint16_t _width, _height;
  uint8_t _textScale;
  enum RA8875sizes _size;
   private:
	volatile bool 			_currentMode;
};

// Colors (RGB565)
#define	RA8875_BLACK            0x0000
#define	RA8875_BLUE             0x001F
#define	RA8875_RED              0xF800
#define	RA8875_GREEN            0x07E0
#define RA8875_CYAN             0x07FF
#define RA8875_MAGENTA          0xF81F
#define RA8875_YELLOW           0xFFE0  
#define RA8875_WHITE            0xFFFF

// Command/Data pins for SPI
#define RA8875_DATAWRITE        0x00
#define RA8875_DATAREAD         0x40
#define RA8875_CMDWRITE         0x80
#define RA8875_CMDREAD          0xC0

// Registers & bits
#define RA8875_PWRR             0x01
#define RA8875_PWRR_DISPON      0x80
#define RA8875_PWRR_DISPOFF     0x00
#define RA8875_PWRR_SLEEP       0x02
#define RA8875_PWRR_NORMAL      0x00
#define RA8875_PWRR_SOFTRESET   0x01

#define RA8875_MRWC             0x02

#define RA8875_GPIOX            0xC7

#define RA8875_PLLC1            0x88
#define RA8875_PLLC1_PLLDIV2    0x80
#define RA8875_PLLC1_PLLDIV1    0x00

#define RA8875_PLLC2            0x89
#define RA8875_PLLC2_DIV1       0x00
#define RA8875_PLLC2_DIV2       0x01
#define RA8875_PLLC2_DIV4       0x02
#define RA8875_PLLC2_DIV8       0x03
#define RA8875_PLLC2_DIV16      0x04
#define RA8875_PLLC2_DIV32      0x05
#define RA8875_PLLC2_DIV64      0x06
#define RA8875_PLLC2_DIV128     0x07

#define RA8875_SYSR             0x10
#define RA8875_SYSR_8BPP        0x00
#define RA8875_SYSR_16BPP       0x0C
#define RA8875_SYSR_MCU8        0x00
#define RA8875_SYSR_MCU16       0x03

#define RA8875_PCSR             0x04
#define RA8875_PCSR_PDATR       0x00
#define RA8875_PCSR_PDATL       0x80
#define RA8875_PCSR_CLK         0x00
#define RA8875_PCSR_2CLK        0x01
#define RA8875_PCSR_4CLK        0x02
#define RA8875_PCSR_8CLK        0x03

#define RA8875_HDWR             0x14

#define RA8875_HNDFTR           0x15
#define RA8875_HNDFTR_DE_HIGH   0x00
#define RA8875_HNDFTR_DE_LOW    0x80

#define RA8875_HNDR             0x16
#define RA8875_HSTR             0x17
#define RA8875_HPWR             0x18
#define RA8875_HPWR_LOW         0x00
#define RA8875_HPWR_HIGH        0x80

#define RA8875_VDHR0            0x19
#define RA8875_VDHR1            0x1A
#define RA8875_VNDR0            0x1B
#define RA8875_VNDR1            0x1C
#define RA8875_VSTR0            0x1D
#define RA8875_VSTR1            0x1E
#define RA8875_VPWR             0x1F
#define RA8875_VPWR_LOW         0x00
#define RA8875_VPWR_HIGH        0x80

#define RA8875_HSAW0            0x30
#define RA8875_HSAW1            0x31
#define RA8875_VSAW0            0x32
#define RA8875_VSAW1            0x33

#define RA8875_HEAW0            0x34
#define RA8875_HEAW1            0x35
#define RA8875_VEAW0            0x36
#define RA8875_VEAW1            0x37

#define RA8875_MCLR             0x8E
#define RA8875_MCLR_START       0x80
#define RA8875_MCLR_STOP        0x00
#define RA8875_MCLR_READSTATUS  0x80
#define RA8875_MCLR_FULL        0x00
#define RA8875_MCLR_ACTIVE      0x40

#define RA8875_DCR                    0x90
#define RA8875_DCR_LINESQUTRI_START   0x80
#define RA8875_DCR_LINESQUTRI_STOP    0x00
#define RA8875_DCR_LINESQUTRI_STATUS  0x80
#define RA8875_DCR_CIRCLE_START       0x40
#define RA8875_DCR_CIRCLE_STATUS      0x40
#define RA8875_DCR_CIRCLE_STOP        0x00
#define RA8875_DCR_FILL               0x20
#define RA8875_DCR_NOFILL             0x00
#define RA8875_DCR_DRAWLINE           0x00
#define RA8875_DCR_DRAWTRIANGLE       0x01
#define RA8875_DCR_DRAWSQUARE         0x10


#define RA8875_ELLIPSE                0xA0
#define RA8875_ELLIPSE_STATUS         0x80

#define RA8875_MWCR0            0x40
#define RA8875_MWCR0_GFXMODE    0x00
#define RA8875_MWCR0_TXTMODE    0x80

#define RA8875_CURH0            0x46
#define RA8875_CURH1            0x47
#define RA8875_CURV0            0x48
#define RA8875_CURV1            0x49

#define RA8875_P1CR             0x8A
#define RA8875_P1CR_ENABLE      0x80
#define RA8875_P1CR_DISABLE     0x00
#define RA8875_P1CR_CLKOUT      0x10
#define RA8875_P1CR_PWMOUT      0x00

#define RA8875_P1DCR            0x8B

#define RA8875_P2CR             0x8C
#define RA8875_P2CR_ENABLE      0x80
#define RA8875_P2CR_DISABLE     0x00
#define RA8875_P2CR_CLKOUT      0x10
#define RA8875_P2CR_PWMOUT      0x00

#define RA8875_P2DCR            0x8D

#define RA8875_PWM_CLK_DIV1     0x00
#define RA8875_PWM_CLK_DIV2     0x01
#define RA8875_PWM_CLK_DIV4     0x02
#define RA8875_PWM_CLK_DIV8     0x03
#define RA8875_PWM_CLK_DIV16    0x04
#define RA8875_PWM_CLK_DIV32    0x05
#define RA8875_PWM_CLK_DIV64    0x06
#define RA8875_PWM_CLK_DIV128   0x07
#define RA8875_PWM_CLK_DIV256   0x08
#define RA8875_PWM_CLK_DIV512   0x09
#define RA8875_PWM_CLK_DIV1024  0x0A
#define RA8875_PWM_CLK_DIV2048  0x0B
#define RA8875_PWM_CLK_DIV4096  0x0C
#define RA8875_PWM_CLK_DIV8192  0x0D
#define RA8875_PWM_CLK_DIV16384 0x0E
#define RA8875_PWM_CLK_DIV32768 0x0F

#define RA8875_TPCR0                  0x70
#define RA8875_TPCR0_ENABLE           0x80
#define RA8875_TPCR0_DISABLE          0x00
#define RA8875_TPCR0_WAIT_512CLK      0x00
#define RA8875_TPCR0_WAIT_1024CLK     0x10
#define RA8875_TPCR0_WAIT_2048CLK     0x20
#define RA8875_TPCR0_WAIT_4096CLK     0x30
#define RA8875_TPCR0_WAIT_8192CLK     0x40
#define RA8875_TPCR0_WAIT_16384CLK    0x50
#define RA8875_TPCR0_WAIT_32768CLK    0x60
#define RA8875_TPCR0_WAIT_65536CLK    0x70
#define RA8875_TPCR0_WAKEENABLE       0x08
#define RA8875_TPCR0_WAKEDISABLE      0x00
#define RA8875_TPCR0_ADCCLK_DIV1      0x00
#define RA8875_TPCR0_ADCCLK_DIV2      0x01
#define RA8875_TPCR0_ADCCLK_DIV4      0x02
#define RA8875_TPCR0_ADCCLK_DIV8      0x03
#define RA8875_TPCR0_ADCCLK_DIV16     0x04
#define RA8875_TPCR0_ADCCLK_DIV32     0x05
#define RA8875_TPCR0_ADCCLK_DIV64     0x06
#define RA8875_TPCR0_ADCCLK_DIV128    0x07

#define RA8875_TPCR1            0x71
#define RA8875_TPCR1_AUTO       0x00
#define RA8875_TPCR1_MANUAL     0x40
#define RA8875_TPCR1_VREFINT    0x00
#define RA8875_TPCR1_VREFEXT    0x20
#define RA8875_TPCR1_DEBOUNCE   0x04
#define RA8875_TPCR1_NODEBOUNCE 0x00
#define RA8875_TPCR1_IDLE       0x00
#define RA8875_TPCR1_WAIT       0x01
#define RA8875_TPCR1_LATCHX     0x02
#define RA8875_TPCR1_LATCHY     0x03

#define RA8875_TPXH             0x72
#define RA8875_TPYH             0x73
#define RA8875_TPXYL            0x74

#define RA8875_INTC1            0xF0
#define RA8875_INTC1_KEY        0x10
#define RA8875_INTC1_DMA        0x08
#define RA8875_INTC1_TP         0x04
#define RA8875_INTC1_BTE        0x02

#define RA8875_INTC2            0xF1
#define RA8875_INTC2_KEY        0x10
#define RA8875_INTC2_DMA        0x08
#define RA8875_INTC2_TP         0x04
#define RA8875_INTC2_BTE        0x02

#endif
