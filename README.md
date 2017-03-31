# RA8875Ada-BTE
Adafruit 1.0.0 old Library with BTE Extension for SPI-Flash

Added Functions

	dispicown(startx,starty,width,height, Startaddress)

	BTE Transfer from Flash@startaddress to Screen startx,starty with size width*height

	void CutPictrue(uchar picnum,uint x1,uint y1,uint x2,uint y2,unsigned long x,unsigned long y);
 	
Change Layer :
  tft.layerEffect(OR);
  
  tft.writeTo(1);
  
  tft.dispicown(0,0, 480,272,0); //Set Background
  
  tft.writeTo(2);
  
  tft.dispicown(100,100, 35,70,0); //Set Foreground
  
	
  
  With BTE_MOVE you can Copy From Layer To Layer or Clipping to Clipping.
  
  if you want Scrolling Combines dispicown and BTE_MOVE
  
	void		BTE_move(int16_t SourceX, int16_t SourceY, int16_t Width, int16_t Height, int16_t DestX, int16_t DestY, uint8_t SourceLayer=0, uint8_t DestLayer=0, bool Transparent = false, uint8_t ROP=RA8875_BTEROP_SOURCE, bool Monochrome=false, bool ReverseDir = false);


Example Scrolltext:

  tft.BTE_move(18, 224, 450, 306, 0, 224, 1, 1);
  // Move The Line 1 Pix To the left and copy over itself
  
  scroller++;
  
  //Every Charwidth Steps add the next visible char At End of Line

if(Scroller>=Charwidthpx)

tft.dispicown(26+(z*18),194, Charwidthpx,Charheigthpx,(uint32_t)1982780+((uint32_t)((uint32_t)firstline[z]-(uint32_t)33)*(uint32_t)1080));

  
  
