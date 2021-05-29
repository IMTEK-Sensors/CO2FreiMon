/******************************************************************************
 * 
 * Basic classes to manage text and groups of text on displays.
 * 
 * Further documentation in .h file
 * 
 * created        14.04.2021
 * last modified  29.05.2021
 * by             Jannik Sehringer
 * for            Laboratory for Sensors,
 *                IMTEK - Department of Microsystems Engineering,
 *                University of Freiburg
 * 
******************************************************************************/

#include "Graphics.h"
#include "bmpDraw.h"    // used in HeaderBar

/******************************************************************************    
*******************************************************************************
    General helper functions
*******************************************************************************
******************************************************************************/

// ____________________________________________________________________________
String dig2(int number) {
  String res = "00";
  res[0] = '0' + (number / 10) % 10;
  res[1] = '0' + number % 10;
  return res;
}

/******************************************************************************    
*******************************************************************************
    Graphics
*******************************************************************************
******************************************************************************/

// init pointer to display object with nullpointer
Adafruit_HX8357* Graphics::_display = NULL;

/******************************************************************************    
*******************************************************************************
    Label
*******************************************************************************
******************************************************************************/

// ____________________________________________________________________________
Label::Label(uint16_t x, uint16_t y, String name, uint8_t size,
             uint16_t color, uint8_t subscript, uint8_t alignment)
    // init all members with the given values
    : _x(x), _y(y), _name(name), _subscript(subscript),
      _size(size), _color(color), _alignment(alignment) {
  correctForAlignment();        // set position according to alignment
  CORRECT_DEGREE_CHAR(_name);   // replace "°" with (char) 248
  print();                      // print the text
}

// ____________________________________________________________________________
Label::Label(uint16_t x, uint16_t y, uint16_t val, uint8_t size,
             uint16_t color, uint8_t alignment)
  // convert value in String with decimal representation, if 0 use " " instead
  : Label(x, y, (val > 0 ? String(val, DEC) : " "), size, color, 0, alignment) {
}

// ____________________________________________________________________________
Label::Label(uint16_t x, uint16_t y, float val, uint8_t size,
             uint16_t color, uint8_t alignment)
  // convert value in String with two decimal places, if 0 use " " instead
  : Label(x, y, (val > 0 ? String(val, 2) : " "), size, color, 0, alignment) {
}

// ____________________________________________________________________________
void Label::print(void) {
  _display->setCursor(_x, _y);      // set position, color and size
  _display->setTextSize(_size);     // of the text to be printed according
  _display->setTextColor(_color);   // to members given in constructor
  if (_subscript) {
    // if a letter is subscripted, print text until subscripted char
    _display->print(_name.substring(0, _subscript - 1));
    int16_t x = _display->getCursorX();       // get cursor position
    int16_t y = _display->getCursorY();
    uint16_t h = 8 * _size;                   // text is (6x8)*textsize pixels
    _display->setCursor(x, y + h/2);          // move cursor half text height down
    _display->setTextSize(_size - 1);         // reduce text size by 1
    _display->print(_name[_subscript - 1]);   // print subscripted char
    x = _display->getCursorX();               // get new x cursor position
    _display->setCursor(x, y);                // move cursor to initial height
    _display->setTextSize(_size);             // set original text size
    _display->print(_name.substring(_subscript));   // print rest of string
  } else {
    // if no letter is subscripted just print the text
    _display->print(_name);
  }
}

// ____________________________________________________________________________
void Label::erase(uint16_t color) {
  // get position, width and height of the text with its text size
  int16_t x, y;
  uint16_t w, h;
  _display->setTextSize(_size);
  _display->getTextBounds(_name, _x, _y, &x, &y, &w, &h);
  if (_subscript) {
    // if a letter is subscripted, get its width and height
    int16_t x1, y1;
    uint16_t w1, h1;
    _display->setTextSize(_size - 1);   // subscripted char is smaller
    _display->getTextBounds(String(_name[_subscript - 1]), _x, _y, &x1, &y1, &w1, &h1);
    h = h/2 + h1;   // it starts at half of the org text height
    w -= CHAR_W;    // subscripted char is one size and thus one char width smaller
  }
  // fill the resulting rectangle with the given color to cover all text
  _display->fillRect(x, y, w, h, color);
}

// ____________________________________________________________________________
void Label::changePosition(int16_t x, int16_t y, uint8_t alignment) {
  _x = x;
  _y = y;
  _alignment = alignment;
  correctForAlignment();
}

// ____________________________________________________________________________
void Label::changeName(String name, uint8_t subscript) {
  CORRECT_DEGREE_CHAR(name);              // replace "°" with (char) 248
  correctForAlignment(name, subscript);   // set position according to alignment
  _name = name;             // set new name
  _subscript = subscript;   // set new subscript
}

// ____________________________________________________________________________
void Label::changeName(uint16_t val) {
  // convert to string and call changeName()
  changeName(String(val, DEC));
}

// ____________________________________________________________________________
void Label::changeName(float val) {
  // convert to string and call changeName()
  changeName(String(val, 2));
}

// ____________________________________________________________________________
void Label::correctForAlignment(void) {
  if ((_alignment & RIGHT) == RIGHT) {
    // number of chars times size times char width is string width in pixels
    _x -= _name.length() * _size * CHAR_W;
    if (_subscript) {
      _x -= CHAR_W;  // subscripted char is one size less
    }
  }
  if ((_alignment & BOTTOM) == BOTTOM) {
    // size times char height is string height in pixels
    _y -= _size * CHAR_H;
  }
  // nothing to do for alignment TOP or LEFT, as that is default of the display
}


// ____________________________________________________________________________
void Label::correctForAlignment(String name, uint8_t subscript) {
  if ((_alignment & RIGHT) == RIGHT) {
    // get difference in length and correct on subscripted chars
    // no difference of length in pixel if both or none contain subscript
    int16_t dif = (name.length() - _name.length()) * _size * CHAR_W;
    // subscript is 1 size smaller, size goes in steps of CHAR_W
    if (!subscript && _subscript) dif += CHAR_W;
    if (subscript && !_subscript) dif -= CHAR_W;

    _x -= dif;  // correct x position
  }
  // no need to correct y as height won't change
}

/******************************************************************************
*******************************************************************************    
    ValueBar
*******************************************************************************
******************************************************************************/

// ____________________________________________________________________________
ValueBar::ValueBar(int16_t x, int16_t y, uint16_t w, uint16_t h,
                   uint16_t color, uint16_t textColor,
                   String name, String unit, uint8_t subscript)
    // init all members with the given values
    : _x(x), _y(y), _w(w), _h(h), _color(color) {
  drawBackground();
  // init all labels, with given name and unit in given textcolor, value empty
  uint8_t size = 4;
  y = _y + (_h + size*CHAR_H) / 2;        // get y position of all labels
  x = _x + 15;                            // get x position of 1. label
  _labels[0] = Label(x, y, name, size-1, textColor, subscript, BOTTOM);
  x = _x + (_w + 5*size*CHAR_W) / 2;      // get x position of 2. label
  _labels[1] = Label(x, y, " ", size, textColor, 0, RIGHT|BOTTOM);
  x = _x + _w - 3*(size-1)*CHAR_W - 10;   // get x position of 3. label
  _labels[2] = Label(x, y, unit, size-1, textColor, 0, BOTTOM);
}

// ____________________________________________________________________________
void ValueBar::drawBackground(void) const {
  // draw the background shape
  _display->fillRoundRect(_x, _y, _w, _h, 10, _color);
}

// ____________________________________________________________________________
void ValueBar::erase(uint16_t color) const {
  // overdraw the background shape with given color
  _display->fillRoundRect(_x, _y, _w, _h, 10, color);
}

// ____________________________________________________________________________
void ValueBar::draw(void) {
  drawBackground();     // draw the background shape
  _labels[0].print();   // print name
  _labels[2].print();   // and unit
}

// ____________________________________________________________________________
void ValueBar::changeColor(uint16_t color) {
  if (color != _color) {  // if color is diffrent from current color
    _color = color;       // set new color
    draw();               // redraw with new color
  }
}

// ____________________________________________________________________________
void ValueBar::refreshValue(uint16_t val) {
  _labels[1].erase(_color);     // overdraw the label text in bg color
  _labels[1].changeName(val);   // change the name to new value
  _labels[1].print();           // print the new label
}

// ____________________________________________________________________________
void ValueBar::refreshValue(float val) {
  _labels[1].erase(_color);     // overdraw the label text in bg color
  _labels[1].changeName(val);   // change the name to new value
  _labels[1].print();           // print the new label
}

/******************************************************************************
*******************************************************************************    
    HeaderBar
*******************************************************************************
******************************************************************************/

// ____________________________________________________________________________
HeaderBar::HeaderBar(int16_t h, uint16_t color, uint16_t textColor,
                     const char* logoFile)
    // init all members with the given values
    : _h(h), _color(color), _logoFile(logoFile), _w(_display->width()) {
  drawBackground();
  uint8_t size = 3;
  h = (_h + size*CHAR_H) / 2;   // get y position of labels
  _date = Label(_w-5, h, (uint16_t) 0, size-1, textColor, RIGHT|BOTTOM);
  _time = Label((_w - 5*size*CHAR_W)/2, h, (uint16_t) 0, size, textColor, BOTTOM);
}

// ____________________________________________________________________________
void HeaderBar::draw(void) {
  drawBackground();
  _time.print();
  _date.print();
}

// ____________________________________________________________________________
void HeaderBar::updateTime(DateTime time) {
  _time.erase(_color);
  _time.changeName(dig2(time.hour()) + ':'
                   + dig2(time.minute()));
  _time.print();
}

// ____________________________________________________________________________
void HeaderBar::updateDate(DateTime date) {
  _date.erase(_color);
  _date.changeName(dig2(date.day()) + '.'
                   + dig2(date.month()) + '.'
                   + date.year());
  _date.print();
}

// ____________________________________________________________________________
int16_t HeaderBar::height(void) const {
  return _h;
}

// ____________________________________________________________________________
void HeaderBar::drawBackground(void) const {
  _display->fillRect(0, 0, _display->width(), _h, _color);
  bmpReader::draw(_logoFile, 1, 1);
}

/******************************************************************************
*******************************************************************************    
    CalibrationWarning
*******************************************************************************
******************************************************************************/

// ____________________________________________________________________________
CalibrationWarning::CalibrationWarning(
  int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color, uint16_t textColor
) : _x(x), _y(y), _w(w), _h(h), _color(color), _textColor(textColor), _textsize(2) {
  // put labels out of visible range, init with correct length
  _co2Name = Label(480, 320, "CO2", _textsize, textColor, 3);
  _co2Value = Label(480, 320, "    ", _textsize, textColor);
  _countdown = Label(480, 320, "     ", _textsize, textColor);
}

// ____________________________________________________________________________
void CalibrationWarning::print(void) {
  // draw background and caption in higher size in the upper center
  _display->fillRoundRect(_x, _y, _w, _h, 10, _color);
  _display->setTextColor(_textColor);
  _display->setTextSize(_textsize+1);
  _display->setCursor(_x + (_w-12*(_textsize+1)*CHAR_W)/2, _y+10);
  _display->println("Calibration!");
  _display->setTextSize(_textsize);
  _display->println();

  int16_t x0 = _x + 10;   // get starting x position of each line
  
  // print instructions on same indent level
  indent(x0);   _display->println("Place device outdoors now!");
  indent(x0);   _display->println("When timer is up, last measured");
  indent(x0);   _display->println("value is set to 417 ppm.");
  indent(x0);   _display->println("Thus sensor must have acclimated");
  indent(x0);   _display->println("to ambient air.");
                _display->println();
  indent(x0);   _display->println("To abort calibration press reset");
  indent(x0);   _display->println("button on upper right backside.");
                _display->println();

  // set position of remaining time label
  indent(x0);   _display->print("Remaining time: ");
  _countdown.changePosition(_display->getCursorX(), _display->getCursorY());
  _countdown.print();                   // must be init with final size (5 chars)
  int16_t x = _display->getCursorX();   // remember x-pos after countdown
  _display->println();

  // set position of CO2 label
  indent(x0);   _display->print("Current ");
  _co2Name.changePosition(_display->getCursorX(), _display->getCursorY());
  _co2Name.print();
  _display->print(": ");
  // use x pos right of countdown to place CO2 label and unit
  _co2Value.changePosition(x, _display->getCursorY(), RIGHT);
  indent(x);    _display->print(" ppm ");
}

// ____________________________________________________________________________
void CalibrationWarning::erase(uint16_t color) const {
  _display->fillRoundRect(_x, _y, _w, _h, 10, color);
}

// ____________________________________________________________________________
void CalibrationWarning::setCalibrationTime(DateTime time) {
  _calibrationTime = time;
}

// ____________________________________________________________________________
DateTime CalibrationWarning::getCalibrationTime(void) const {
  return _calibrationTime;
}

// ____________________________________________________________________________
void CalibrationWarning::refreshCountdown(DateTime time) {
  TimeSpan remaining = _calibrationTime - time;
  _countdown.erase(_color);
  _countdown.changeName(dig2(remaining.minutes()) + ':'
                        + dig2(remaining.seconds()));
  _countdown.print();
}

// ____________________________________________________________________________
void CalibrationWarning::refreshCO2(uint16_t val) {
  _co2Value.erase(_color);
  _co2Value.changeName(val);
  _co2Value.print();
}

// ____________________________________________________________________________
void CalibrationWarning::indent(int16_t x) const {
  _display->setCursor(x, _display->getCursorY());
}