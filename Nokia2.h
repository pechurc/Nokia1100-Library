#ifndef Nokia2_h
#define Nokia2_h

#include "Arduino.h"
#include "Print.h"

#define PIN_SCE 3 // 2 XCS 
#define PIN_SDIN 4 // 4 SDA
#define PIN_RESET 5 // 1 XRES
#define PIN_SCLK 6 // 5 SCLK
#define PIN_BL 2 // 8 BackLight

#define LCD_C LOW // Command
#define LCD_D HIGH // Data

#define LCD_X 96
#define LCD_Y 65
#define Cache_Size 96 * 9

#define SetYAddr 0xB0
#define SetXAddr4 0x00
#define SetXAddr3 0x10

#define swap(a, b) { int16_t t = a; a = b; b = t; }

class LCD : public Print
{
	public:
		virtual size_t write(uint8_t);
		void Clear(void);
		void WriteLCD(byte dc, byte data);
		void Setup();
		void Update();
		
		virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color);
		virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
		virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
		virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
		virtual void fillScreen(uint16_t color);
		void drawPixel(int16_t x, int16_t y, uint16_t color);
		void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
		void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
		void drawLine(int16_t x0, int16_t y0,int16_t x1, int16_t y1, uint16_t color);
		void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
		void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
		void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
		void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);
		void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
		void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
		void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
		void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);

		void setCursor(int16_t x, int16_t y);
		void setTextColor(uint16_t c);
		void setTextColor(uint16_t c, uint16_t bg);
		void setTextSize(uint8_t s);
		
	private:
	void UpdateBox (int xmin, int ymin, int xmax, int ymax);
	void Initialise();	
	boolean wrap;
	uint16_t textcolor, textbgcolor;
	uint8_t textsize;
};

#endif