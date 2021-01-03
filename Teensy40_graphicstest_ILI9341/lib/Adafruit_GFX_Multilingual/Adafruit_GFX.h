#ifndef _ADAFRUIT_GFX_H
#define _ADAFRUIT_GFX_H

#include "glcdfont.h"

#if ARDUINO >= 100
 #include "Arduino.h"
 #include "Print.h"
#else
 #include "WProgram.h"
#endif
#include "gfxfont.h"

//#define DEBUG_ADAFRUIT_GFX_MULTILINGUAL

#define UNIFONT_USE_SDFAT

// NOTE: I only have the Feather M0 Express, M4 Express and PyPortal boards to test with.
#if defined(ADAFRUIT_FEATHER_M0_EXPRESS) /* UNTESTED: */ || defined(ADAFRUIT_METRO_M0_EXPRESS) || defined(ADAFRUIT_CIRCUITPLAYGROUND_M0) || defined(ADAFRUIT_ITSYBITSY_M0) || defined(ADAFRUIT_HALLOWING)
 #define UNIFONT_USE_FLASH
 #define UNIFONT_USE_SPI
#endif

#if defined(ADAFRUIT_FEATHER_M4_EXPRESS) || defined(ADAFRUIT_PYPORTAL) /* UNTESTED: */ || defined(ADAFRUIT_TRELLIS_M4_EXPRESS) || defined(ADAFRUIT_GRAND_CENTRAL_M4) || defined(ADAFRUIT_ITSYBITSY_M4_EXPRESS) || defined(ADAFRUIT_METRO_M4_EXPRESS)
 #define UNIFONT_USE_FLASH
 #define UNIFONT_USE_QSPI
#endif

#ifdef UNIFONT_USE_FLASH
 #include <SPI.h>
 #include <Adafruit_SPIFlash.h>
 #include <Adafruit_SPIFlash_FatFs.h>

 #ifdef UNIFONT_USE_QSPI
  #include <Adafruit_QSPI_GD25Q.h>
 #endif // UNIFONT_USE_QSPI
#endif // UNIFONT_USE_FLASH

#ifdef UNIFONT_USE_SDFAT
 #include <SdFat.h>
#endif

typedef enum _encoding {
	none = 0,	// ISO-8859-1
	utf8		// UTF-8
} encoding_t;

/// A generic graphics superclass that can handle all sorts of drawing. At a minimum you can subclass and provide drawPixel(). At a maximum you can do a ton of overriding to optimize. Used for any/all Adafruit displays!
class Adafruit_GFX : public Print {

 public:

  Adafruit_GFX(int16_t w, int16_t h); // Constructor

  // This MUST be defined by the subclass:
  virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;    ///< Virtual drawPixel() function to draw to the screen/framebuffer/etc, must be overridden in subclass. @param x X coordinate.  @param y Y coordinate. @param color 16-bit pixel color. 

  // TRANSACTION API / CORE DRAW API
  // These MAY be overridden by the subclass to provide device-specific
  // optimized code.  Otherwise 'generic' versions are used.
  virtual void startWrite(void);
  virtual void writePixel(int16_t x, int16_t y, uint16_t color);
  virtual void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  virtual void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  virtual void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
  virtual void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
  virtual void endWrite(void);

  // CONTROL API
  // These MAY be overridden by the subclass to provide device-specific
  // optimized code.  Otherwise 'generic' versions are used.
  virtual void setRotation(uint8_t r);
  virtual void invertDisplay(boolean i);

  // BASIC DRAW API
  // These MAY be overridden by the subclass to provide device-specific
  // optimized code.  Otherwise 'generic' versions are used.
  virtual void
    // It's good to implement those, even if using transaction API
    drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color),
    drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color),
    fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
    fillScreen(uint16_t color),
    // Optional and probably not necessary to change
    drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color),
    drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

  // Unifont support (Static members and functions)
#if defined(UNIFONT_USE_FLASH) || defined(UNIFONT_USE_SDFAT)
  static void loadUnifontFile(const char *dir, const char *file);
  static boolean unifileavailable;
  static UnifontBlock *unifont;
#ifdef UNIFONT_USE_FLASH
  static File unifile;    // file handle to unifont.bin, if available
#endif
#ifdef UNIFONT_USE_SDFAT
  static MutexFsBaseFile unifile;    // file handle to unifont.bin, if available
#endif
#endif // if defined(UNIFONT_USE_FLASH) || defined(UNIFONT_USE_SDFAT)

  // These exist only with Adafruit_GFX (no subclass overrides)
  void
    drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color),
    drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
      uint16_t color),
    fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color),
    fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
      int16_t delta, uint16_t color),
    drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
      int16_t x2, int16_t y2, uint16_t color),
    fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
      int16_t x2, int16_t y2, uint16_t color),
    drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h,
      int16_t radius, uint16_t color),
    fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h,
      int16_t radius, uint16_t color),
    drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[],
      int16_t w, int16_t h, uint16_t color),
    drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[],
      int16_t w, int16_t h, uint16_t color, uint16_t bg),
    drawBitmap(int16_t x, int16_t y, uint8_t *bitmap,
      int16_t w, int16_t h, uint16_t color),
    drawBitmap(int16_t x, int16_t y, uint8_t *bitmap,
      int16_t w, int16_t h, uint16_t color, uint16_t bg),
    drawXBitmap(int16_t x, int16_t y, const uint8_t bitmap[],
      int16_t w, int16_t h, uint16_t color),
    drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t bitmap[],
      int16_t w, int16_t h),
    drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t *bitmap,
      int16_t w, int16_t h),
    drawGrayscaleBitmap(int16_t x, int16_t y,
      const uint8_t bitmap[], const uint8_t mask[],
      int16_t w, int16_t h),
    drawGrayscaleBitmap(int16_t x, int16_t y,
      uint8_t *bitmap, uint8_t *mask, int16_t w, int16_t h),
    drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[],
      int16_t w, int16_t h),
    drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap,
      int16_t w, int16_t h),
    drawRGBBitmap(int16_t x, int16_t y,
      const uint16_t bitmap[], const uint8_t mask[],
      int16_t w, int16_t h),
    drawRGBBitmap(int16_t x, int16_t y,
      uint16_t *bitmap, uint8_t *mask, int16_t w, int16_t h),
    drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color,
      uint16_t bg, uint8_t size),
    setCursor(int16_t x, int16_t y),
    setTextColor(uint16_t c),
    setTextColor(uint16_t c, uint16_t bg),
    setTextSize(uint8_t s),
    setFont(const GFXfont *f = NULL, int16_t ofs_y = 0),
    setTextWrap(boolean w),
    setRTL(boolean r),
    cp437(boolean x=true),
    getTextBounds(const char *string, int16_t x, int16_t y,
      int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h, encoding_t encoding = none),
    getTextBounds(const __FlashStringHelper *s, int16_t x, int16_t y,
      int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h, encoding_t encoding = none),
    getTextBounds(const String &str, int16_t x, int16_t y,
      int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h, encoding_t encoding = none);
  int
    drawCodepoint(int16_t x, int16_t y, uint16_t c, uint16_t color,
      uint16_t bg, uint8_t size, bool doDraw = true);

  size_t
    writeCodepoint(uint16_t c),
    print(const char *string, encoding_t encoding = none),
    println(const char *string, encoding_t encoding = none);
    /*
    printUTF8(const char *string),
    printlnUTF8(const char *string);
    */


#if ARDUINO >= 100
  virtual size_t write(uint8_t);
#else
  virtual void   write(uint8_t);
#endif

  int16_t height(void) const;
  int16_t width(void) const;

  uint8_t getRotation(void) const;

  // get current cursor position (get rotation safe maximum values, using: width() for x, height() for y)
  int16_t getCursorX(void) const;
  int16_t getCursorY(void) const;

 protected:
  void
    charBounds(char c, int16_t *x, int16_t *y,
      int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy),
    codepointBounds(uint16_t c, int16_t *x, int16_t *y,
      int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy);
  const int16_t
    WIDTH,          ///< This is the 'raw' display width - never changes
    HEIGHT;         ///< This is the 'raw' display height - never changes
  int16_t
    _width,         ///< Display width as modified by current rotation
    _height,        ///< Display height as modified by current rotation
    cursor_x,       ///< x location to start print()ing text
    cursor_y;       ///< y location to start print()ing text
  uint16_t
    textcolor,      ///< 16-bit background color for print()
    textbgcolor;    ///< 16-bit text color for print()
  uint8_t
    textsize,       ///< Desired magnification of text to print()
    rotation;       ///< Display rotation (0 thru 3)
  int8_t direction; ///< 1 for LTR, -1 for RTL.
  boolean
    wrap;           ///< If set, 'wrap' text at right edge of display
  GFXfont
    *gfxFont;       ///< Pointer to special font
  int16_t
    gfxFont_ofs_y;  ///< Y offset for special font
 private:
  inline uint8_t index_for_block(uint8_t block);
  void fix_diacritics(uint16_t *s, size_t length);
};


/// A simple drawn button UI element
class Adafruit_GFX_Button {

 public:
  Adafruit_GFX_Button(void);
  // "Classic" initButton() uses center & size
  void initButton(Adafruit_GFX *gfx, int16_t x, int16_t y,
   uint16_t w, uint16_t h, uint16_t outline, uint16_t fill,
   uint16_t textcolor, char *label, uint8_t textsize);
  // New/alt initButton() uses upper-left corner & size
  void initButtonUL(Adafruit_GFX *gfx, int16_t x1, int16_t y1,
   uint16_t w, uint16_t h, uint16_t outline, uint16_t fill,
   uint16_t textcolor, char *label, uint8_t textsize);
  void drawButton(boolean inverted = false);
  boolean contains(int16_t x, int16_t y);

  void press(boolean p);
  boolean isPressed();
  boolean justPressed();
  boolean justReleased();

 private:
  Adafruit_GFX *_gfx;
  int16_t       _x1, _y1; // Coordinates of top-left corner
  uint16_t      _w, _h;
  uint8_t       _textsize;
  uint16_t      _outlinecolor, _fillcolor, _textcolor;
  char          _label[10];

  boolean currstate, laststate;
};


/// A GFX 1-bit canvas context for graphics
class GFXcanvas1 : public Adafruit_GFX {
 public:
  GFXcanvas1(uint16_t w, uint16_t h);
  ~GFXcanvas1(void);
  void     drawPixel(int16_t x, int16_t y, uint16_t color),
           fillScreen(uint16_t color);
  uint8_t *getBuffer(void);
 private:
  uint8_t *buffer;
};


/// A GFX 8-bit canvas context for graphics
class GFXcanvas8 : public Adafruit_GFX {
 public:
  GFXcanvas8(uint16_t w, uint16_t h);
  ~GFXcanvas8(void);
  void     drawPixel(int16_t x, int16_t y, uint16_t color),
           fillScreen(uint16_t color),
           writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

  uint8_t *getBuffer(void);
 private:
  uint8_t *buffer;
};


///  A GFX 16-bit canvas context for graphics
class GFXcanvas16 : public Adafruit_GFX {
 public:
  GFXcanvas16(uint16_t w, uint16_t h);
  ~GFXcanvas16(void);
  void      drawPixel(int16_t x, int16_t y, uint16_t color),
            fillScreen(uint16_t color);
  uint16_t *getBuffer(void);
 private:
  uint16_t *buffer;
};

#endif // _ADAFRUIT_GFX_H