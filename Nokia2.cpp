#include "Arduino.h"
#include "Nokia2.h"
#include "NokiaChars.h"
#include <avr/pgmspace.h>

byte LCDCache [Cache_Size];
static int xUpdateMin, yUpdateMin, xUpdateMax,yUpdateMax;
static int Cursor_X, Cursor_Y;

void LCD::Setup() {
  pinMode(PIN_SCE, OUTPUT);
  pinMode(PIN_RESET, OUTPUT);
  pinMode(PIN_SDIN, OUTPUT);
  pinMode(PIN_SCLK, OUTPUT);
  pinMode(PIN_BL, OUTPUT);
  digitalWrite(PIN_BL, HIGH);
  digitalWrite(PIN_RESET, LOW);
  digitalWrite(PIN_SCE, HIGH);
  digitalWrite(PIN_SCLK, LOW);
  wrap = true;
  textcolor = 0xFF;
  textbgcolor = 0x00;
  textsize = 1;
  Initialise();
}

void LCD::Clear() {
	for (int index = 0; index < 864 ; index++)
  {
   LCDCache[index] = (0x00);
  }
  UpdateBox(0, 0, LCD_X - 1, LCD_Y - 1); 
}

void LCD::Initialise() {
  digitalWrite(PIN_RESET, LOW);
  digitalWrite(PIN_RESET, HIGH);
  WriteLCD(LCD_C, 0x23); // CONTRAST?
  WriteLCD(LCD_C, 0x2F); // Charge pump ON
  WriteLCD(LCD_C, 0x24); // Vop MSB *Don't change*
  WriteLCD(LCD_C, 0x80); // Vop LSB *Here you can change contrast*
  WriteLCD(LCD_C, 0xA4); // A4 = normal display mode, A5 = all pixels ON
  WriteLCD(LCD_C, 0xAF); // Display ON
}

void LCD::WriteLCD(byte dc, byte data) {
  digitalWrite(PIN_SDIN, dc); // dc is sampled with the first rising SCLK edge
  digitalWrite(PIN_SCE, LOW); // LCD enable
  digitalWrite(PIN_SCLK, HIGH); // First rising SCLK edge
  digitalWrite(PIN_SCLK, LOW);
  shiftOut(PIN_SDIN, PIN_SCLK, MSBFIRST, data); // SDIN is: sampled at the rising edge of SCLK.
  digitalWrite(PIN_SCE, HIGH);
 }

void LCD::UpdateBox (int xmin, int ymin, int xmax, int ymax) {
	if (xmin < xUpdateMin) xUpdateMin = xmin;
	if (xmax > xUpdateMax) xUpdateMax = xmax;
	if (ymin < yUpdateMin) yUpdateMin = ymin;
	if (ymax > yUpdateMax) yUpdateMax = ymax;
}

void LCD::drawPixel (int16_t x, int16_t y, uint16_t color) {
	if ((x < 0) || (x >= LCD_X) || (y < 0) || (y >= LCD_Y))
	return;
	
	if (color)
    LCDCache[x+ (y/8)*LCD_X] |= _BV(y%8);
  else
    LCDCache[x+ (y/8)*LCD_X] &= ~_BV(y%8); 
	
	UpdateBox(x,y,x,y);
}

 void LCD::Update() {
	for(int p = 0; p < 9; p++){
		if(yUpdateMin >= ((p+1) *8)) {
			continue;
		}
		if(yUpdateMax < p*8){
				break;
		}
		WriteLCD(LCD_C, SetYAddr | p); 
		
		int col = xUpdateMin;
		int maxcol = xUpdateMax;
			
		WriteLCD(LCD_C, SetXAddr4 | (col - (16 * (col/16))));
		WriteLCD(LCD_C, SetXAddr3 | (col / 16));
		
		for(; col <= maxcol; col++){
			WriteLCD(LCD_D, LCDCache[(LCD_X * p) + col]);
		}
		
	}
	xUpdateMin = LCD_X - 1;	
	xUpdateMax = 0;
	yUpdateMin = LCD_Y -1;
	yUpdateMax = 0;
}
 
size_t LCD::write(uint8_t c) {
 if (c == '\n') {
    Cursor_Y += 8;
    Cursor_X = 0;
  } else if (c == '\r') {
    // skip em
  } else {
    drawChar(Cursor_X, Cursor_Y, c, textcolor, textbgcolor, textsize);
    Cursor_X += 6;
    if (wrap && (Cursor_X > (LCD_X - 6))) {
      Cursor_Y += 8;
      Cursor_X = 0;
    }
  }
  return 1;
}

void LCD::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {

  if((x >= LCD_X) ||(y >= LCD_Y) || ((x + 5 * size - 1) < 0) || ((y + 8 * size - 1) < 0))
    return;

  for (int8_t i=0; i<6; i++ ) {
    uint8_t line;
    if (i == 5)
      line = 0x0;
    else
      line = pgm_read_byte(font+(c*5)+i);
    for (int8_t j = 0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, color);
        else { // big size
          fillRect(x+(i*size), y+(j*size), size, size, color);
        }
      } else if (bg != color) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, bg);
        else { // big size
          fillRect(x+i*size, y+j*size, size, size, bg);
        }
      }
      line >>= 1;
    }
  }
}

void LCD::drawLine(int16_t x0, int16_t y0,int16_t x1, int16_t y1, uint16_t color) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void LCD::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  for (int16_t i=x; i<x+w; i++) {
    drawFastVLine(i, y, h, color);
  }
}

void LCD::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
  drawLine(x, y, x, y+h-1, color);
}

void LCD::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
  drawLine(x, y, x+w-1, y, color);
}

void LCD::setTextColor(uint16_t c) {
  textcolor = c;
  textbgcolor = c;
}

 void LCD::setTextColor(uint16_t c, uint16_t b) {
   textcolor = c;
   textbgcolor = b;
 }

 void LCD::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
  for (int16_t j=0; j<h; j++) {
    for (int16_t i=0; i<w; i++ ) {
      if (pgm_read_byte(bitmap + i + (j/8)*w) & _BV(j%8)) {
drawPixel(x+i, y+j, color);
      }
    }
  }
}

void LCD::setTextSize(uint8_t s) {
  textsize = (s > 0) ? s : 1;
}

void LCD::setCursor(int16_t x, int16_t y) {
  Cursor_X = x;
  Cursor_Y = y;
}

void LCD::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  drawFastHLine(x, y, w, color);
  drawFastHLine(x, y+h-1, w, color);
  drawFastVLine(x, y, h, color);
  drawFastVLine(x+w-1, y, h, color);
}

void LCD::drawCircle(int16_t x0, int16_t y0, int16_t r,
uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  drawPixel(x0, y0+r, color);
  drawPixel(x0, y0-r, color);
  drawPixel(x0+r, y0, color);
  drawPixel(x0-r, y0, color);

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);
    
  }
}

void LCD::drawCircleHelper( int16_t x0, int16_t y0,
               int16_t r, uint8_t cornername, uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    if (cornername & 0x4) {
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 + y, y0 + x, color);
    }
    if (cornername & 0x2) {
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      drawPixel(x0 - y, y0 - x, color);
      drawPixel(x0 - x, y0 - y, color);
    }
  }
}

void LCD::fillCircle(int16_t x0, int16_t y0, int16_t r,
uint16_t color) {
  drawFastVLine(x0, y0-r, 2*r+1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}

void LCD::fillCircleHelper(int16_t x0, int16_t y0, int16_t r,
uint8_t cornername, int16_t delta, uint16_t color) {

  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    if (cornername & 0x1) {
      drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
      drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
    }
    if (cornername & 0x2) {
      drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
      drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
    }
  }
}

void LCD::fillScreen(uint16_t color) {
  fillRect(0, 0, LCD_X, LCD_Y, color);
}

void LCD::drawRoundRect(int16_t x, int16_t y, int16_t w,
  int16_t h, int16_t r, uint16_t color) {
  // smarter version
  drawFastHLine(x+r , y , w-2*r, color); // Top
  drawFastHLine(x+r , y+h-1, w-2*r, color); // Bottom
  drawFastVLine( x , y+r , h-2*r, color); // Left
  drawFastVLine( x+w-1, y+r , h-2*r, color); // Right
  // draw four corners
  drawCircleHelper(x+r , y+r , r, 1, color);
  drawCircleHelper(x+w-r-1, y+r , r, 2, color);
  drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
  drawCircleHelper(x+r , y+h-r-1, r, 8, color);
}

void LCD::fillRoundRect(int16_t x, int16_t y, int16_t w,
int16_t h, int16_t r, uint16_t color) {
  // smarter version
  fillRect(x+r, y, w-2*r, h, color);

  // draw four corners
  fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
  fillCircleHelper(x+r , y+r, r, 2, h-2*r-1, color);
}

void LCD::drawTriangle(int16_t x0, int16_t y0,
int16_t x1, int16_t y1,
int16_t x2, int16_t y2, uint16_t color) {
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}

void LCD::fillTriangle ( int16_t x0, int16_t y0,
int16_t x1, int16_t y1,
int16_t x2, int16_t y2, uint16_t color) {

  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }
  if (y1 > y2) {
    swap(y2, y1); swap(x2, x1);
  }
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }

  if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a) a = x1;
    else if(x1 > b) b = x1;
    if(x2 < a) a = x2;
    else if(x2 > b) b = x2;
    drawFastHLine(a, y0, b-a+1, color);
    return;
  }

  int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1,
    sa = 0,
    sb = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2. If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2) last = y1; // Include y1 scanline
  else last = y1-1; // Skip it

  for(y=y0; y<=last; y++) {
    a = x0 + sa / dy01;
    b = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
*/
    if(a > b) swap(a,b);
    drawFastHLine(a, y, b-a+1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2. This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++) {
    a = x1 + sa / dy12;
    b = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
*/
    if(a > b) swap(a,b);
    drawFastHLine(a, y, b-a+1, color);
  }
}