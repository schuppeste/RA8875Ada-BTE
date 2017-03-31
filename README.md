# RA8875Ada-BTE
Adafruit 1.0.0 old Library with BTE Extension for SPI-Flash

Added Function 
dispicown(startx,starty,width,height, Startaddress)
BTE Transfer from Flash@startaddress to Screen startx,starty with size width*height

void CutPictrue(uchar picnum,uint x1,uint y1,uint x2,uint y2,unsigned long x,unsigned long y);
 	void	 	BTE_moveFrom(int16_t SX,int16_t SY);
	void	 	BTE_moveTo(int16_t DX,int16_t DY);
	void		BTE_ropcode(unsigned char setx);//
	void 		BTE_enable(bool on);//
	void 		BTE_dataMode(enum RA8875btedatam m);//CONT,RECT
	void 		BTE_layer(enum RA8875btelayer sd,uint8_t l);//SOURCE,DEST - 1 or 2
  
  With BTE_MOVE you can Copy From Layer To LAyer or Clipping to Clipping.
  
	void		BTE_move(int16_t SourceX, int16_t SourceY, int16_t Width, int16_t Height, int16_t DestX, int16_t DestY, uint8_t SourceLayer=0, uint8_t DestLayer=0, bool Transparent = false, uint8_t ROP=RA8875_BTEROP_SOURCE, bool Monochrome=false, bool ReverseDir = false);
