/******************************************************************************
 * 
 * Basic classes to manage text and groups of text on displays.
 * 
 * - A Graphics class to take a display instance once and hold it as static.
 * Other classes derived from this can use the _display member to print on.
 * - A class to print text on a display with additional options to change
 * and erase it.
 * - A class to print measurement values between a name and a unit, with
 * options to change value and color or erase it.
 * - A class to print a header bar containing updatable date and time
 * and a logo.
 * - A class to show information and instructions about a pending calibration
 * of a sensor.
 * 
 * Note:
 *  While class Label is pretty much generic for usage in different kinds
 *  of application, the other classes are quite specific for the task
 *  of the CO2 monitor built.
 * 
 * Circuit:
 *  - Adafruit TFT FeatherWing - 3,5" 480x320
 *      other displays might work to, provided there is a library derived
 *      from "Adafruit_GFX" to controll them. Make sure to change
 *      DISPLAY_TYPE accordingly.
 * 
 * created        14.04.2021
 * last modified  29.05.2021
 * by             Jannik Sehringer
 * for            Laboratory for Sensors,
 *                IMTEK - Department of Microsystems Engineering,
 *                University of Freiburg
 * 
******************************************************************************/

#ifndef _GRAPHICS__H_
#define _GRAPHICS__H_

#include <Adafruit_HX8357.h>
#include <RTClib.h>

// dimensions of charaters in pixels
// characters on screen have these dimensions times the textsize
#define CHAR_W  6
#define CHAR_H  8

/* Class of the used display (recommended: Adafruit_HX8357)
 * must have the following methods:
 *  setCursor(int16_t, int16_t)
 *  setTextSize(uint8_t)
 *  setTextColor(uint16_t)
 *  print(const String&)
 *  print(char)
 *  fillRect(int16_t, int16_t, int16_t, int16_t, uint16_t)
 *  fillRoundRect(int16_t, int16_t, int16_t, int16_t, int16_t, uint16_t)
 *  int16_t getCursorX()
 *  int16_t getCursorY()
 *  getTextBounds(const String&, int16_t, int16_t, int16_t, int16_t, uint16_t, uint16_t)
 * This should account for all types derived from "Adafruit_GFX",
 * for use of bmpReader it must be a type derived from "Adafruit_SPITFT" */
#define DISPLAY_TYPE   Adafruit_HX8357

/* Representation of "°" in String takes two bytes and will produce two
 * characters when printed with HX3857, to avoid that replace it with the
 * ASCII char 248 if present. */
#define CORRECT_DEGREE_CHAR(str) \
 if (str.indexOf("°") >= 0) str.replace("°", String((char) 248))

// options for justification of text
// use alignment in both directions with bitwise or operator "|"
// BOTTOM is usefull for labels of different size to appear on same baseline
#define LEFT    0x0   // default
#define RIGHT   0x1
#define TOP     0x0   // default
#define BOTTOM  0x2

/******************************************************************************    
*******************************************************************************
    General helper functions
*******************************************************************************
******************************************************************************/

// convert integer to two digit string with leading zeros
String dig2(int number);

/*****************************************************************************    
******************************************************************************
    Graphics
******************************************************************************
*****************************************************************************/

/* Simple class to hold a static display instance to be initialized once and
 * used by all graphic subclasses. */
class Graphics {
 public:
  /* Methods */
  // takes the reference to the display instance and stores it
  static void useDisplay(DISPLAY_TYPE* display) {_display = display;}
 protected:
  /* Members */
  static DISPLAY_TYPE* _display;  ///< pointer to instance of display
};

/*****************************************************************************    
******************************************************************************
    Label
******************************************************************************
*****************************************************************************/

/* Graphic class of text on screen with options to replace and erase it. */
class Label : public Graphics {
 public:
  /* Methods */
  // constructor with text given as String
  // take text, position (x, y), size, color and possibly subscript position
  Label(uint16_t x, uint16_t y, String name, uint8_t size, uint16_t color,
        uint8_t subscript = 0, uint8_t alignment=TOP|LEFT);
  // constructor where text is a value given as uint16_t
  Label(uint16_t x, uint16_t y, uint16_t val, uint8_t size, uint16_t color,
        uint8_t alignment=TOP|LEFT);
  // constructor where text is a value given as float
  Label(uint16_t x, uint16_t y, float val, uint8_t size, uint16_t color,
        uint8_t alignment=TOP|LEFT);
  // empty default constructor
  Label(void) {}

  // print the text at the with size and color at position
  void print(void);
  // take smallest rectangle covering the text and fill it with the given color
  void erase(uint16_t color);

  /* the following methods only change the internal value, make sure to erase
   * the Label before and print it afterwards to make change visible */
  // change the position of the Label
  void changePosition(int16_t x, int16_t y, uint8_t alignment=TOP|LEFT);
  // change the text of the Label
  void changeName(String name, uint8_t subscript = 0);
  void changeName(uint16_t val);
  void changeName(float val);

 private:
  /* Methods */
  // adjust upper left corner of text according to alignment...
  // ... on set of position
  void correctForAlignment(void);
  // ... on change of name
  void correctForAlignment(String name, uint8_t subscript);

  /* Members */
  uint16_t _x, _y;      ///< upper left corner of the label
  uint16_t _color;      ///< textcolor
  uint8_t _size;        ///< textsize
  String _name;         ///< actual text to be printed
  uint8_t _subscript;   ///< index of subscripted char +1, if 0 no subscript
  uint8_t _alignment;   ///< alignment of text
};

/*****************************************************************************    
******************************************************************************
    ValueBar
******************************************************************************
*****************************************************************************/

/* Class to draw colored bars with a name, a value and an unit. */
class ValueBar : public Graphics {
 public:
  /* Methods */
  ValueBar(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color,
           uint16_t textColor, String name, String unit, uint8_t subscript = 0);

  // overdraw shape width given color
  void erase(uint16_t color) const;
  // draw background and reprint name and unit labels
  // call refreshValue() afterwards otherwise no value will be shown
  void draw(void);
  // change the background color and reprint using draw()
  void changeColor(uint16_t color);
  // change the value label
  void refreshValue(uint16_t val);
  void refreshValue(float val);

 private:
  /* Methods */
  // print the background shape
  void drawBackground(void) const;
  
  /* Members */
  int16_t _x, _y;     ///< upper left corner of bar
  uint16_t _w, _h;    ///< width and height of the bar
  uint16_t _color;    ///< backround color of the bar
  Label _labels[3];   ///< 3 labels: name, value and unit
};

/*****************************************************************************    
******************************************************************************
    HeaderBar
******************************************************************************
*****************************************************************************/

/* Class to draw a header bar showing a logo, time and date. */
class HeaderBar : public Graphics {
 public:
  /* Methods */
  HeaderBar(int16_t h, uint16_t color, uint16_t textColor, const char* logoFile);
  
  // draw background and reprint date and time labels
  void draw(void);
  // show the given time on the display
  void updateTime(DateTime time);
  // show the given date on the display
  void updateDate(DateTime date);
  // return the height of the bar
  int16_t height(void) const;
  
 private:
  /* Methods */
  // draw the background shape including the logo
  void drawBackground(void) const;
  
  /* Members */
  int16_t _w, _h;         ///< width and height of the bar
  uint16_t _color;        ///< backround color of the bar
  const char* _logoFile;  ///< filename of the logo to draw
  Label _date;            ///< label to show time
  Label _time;            ///< label to show date
};

/* Class to print out information about a pending calibration. */
class CalibrationWarning : public Graphics {
 public:
  CalibrationWarning(int16_t x, int16_t y, uint16_t w, uint16_t h,
                     uint16_t color, uint16_t textColor);
  
  // draw new background shape and print calibration warning
  void print(void);
  // overdraw shape width given color
  void erase(uint16_t color) const;
  // set time for calibration to be executed
  void setCalibrationTime(DateTime time);
  // get the time calibration is scheduled for
  DateTime getCalibrationTime(void) const;
  // update the countdown till calibration with new time
  void refreshCountdown(DateTime time);
  // update current CO2 value with new value
  void refreshCO2(uint16_t co2);

 private:
  /* Methods */
  // indent cursor given amount of pixels in x direction
  inline void indent(int16_t x) const;
  
  /* Members */
  int16_t _x, _y;               ///< upper left corner
  uint16_t _w, _h;              ///< width and height
  uint16_t _color, _textColor;  ///< color of background and text
  uint8_t _textsize;            ///< size of the text
  Label _co2Name;               ///< subscripted name CO2
  Label _co2Value;              ///< value of co2
  Label _countdown;             ///< remaining time
  DateTime _calibrationTime;    ///< time of calibration
};

#endif  // _GRAPHICS__H_