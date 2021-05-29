/******************************************************************************
 * 
 * Read bmp files and print them on display.
 * 
 * A function to read a bmp file from an SD card and directly print it on
 * a display with its helper functions to read data from files put together
 * in a class. The class used to controll the display must be derived from
 * "Adafruit_SPITFT".
 * 
 * Circuit:
 *  - Adafruit TFT FeatherWing - 3,5" 480x320
 *      other displays might work to, provided there is a library derived
 *      from "Adafruit_SPITFT" to controll them. Make sure to change
 *      DISPLAY_TYPE accordingly.
 *  - A SD-card socket
 *      is provided on the Adafruit TFT FeatherWing - 3,5" 480x320
 * 
 * created        14.04.2021
 * last modified  29.05.2021
 * by             Jannik Sehringer (adapted from Adafruit example code)
 * for            Laboratory for Sensors,
 *                IMTEK - Department of Microsystems Engineering,
 *                University of Freiburg
 * 
 * /***************************************************
  This is our library for the Adafruit HX8357D FeatherWing
  ----> http://www.adafruit.com/products/3651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************
 * 
 * This code was adapted from Adafruit and slightly modified.
 * The original code can be found in the example
 * "bitmapdraw_feahterwing.ino" in the Adafruit_HX8357_Library.
 * 
******************************************************************************/

#ifndef _BMP_DRAW__H_
#define _BMP_DRAW__H_

#include <Adafruit_HX8357.h>
#include <SD.h>
#include "Graphics.h"


// The main function opens a Windows Bitmap (BMP) file
// and displays it at the given coordinates.  It's sped
// up by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  50 pixels seems a
// good balance.
#define BUFFPIXEL 50

class bmpReader : public Graphics {
 public:
  // draw the bmp file of given name on display position (x, y)
  static void draw(const char* filename, int16_t x, int16_t y);

 private:
  // read 2 bytes from the given file
  static uint16_t read16(File &f);
  // read 4 bytes from the given file
  static uint32_t read32(File &f);
};

#endif  // _BMP_DRAW__H_