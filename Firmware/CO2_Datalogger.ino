/******************************************************************************
 * 
 * Real time CO2 datalogger with GUI.
 * 
 * Logging data from SCD30 CO2 sensor on a SD card with timestamps using a
 * DS3231 RTC. Visualize measurement data on a 3.5" TFT display.
 * Create a new data file for every day.
 * 
 * Circuit:
 *  - Adafruit Feather M0
 *  - Adafruit DS3231 Precision RTC FeatherWing
 *  - Adafruit TFT FeatherWing - 3,5" 480x320
 *  - SDC30 CO2 sensor (I2C-address 0x61)
 *  - Pushbutton, connected between GND and pin 14 (A0)
 * 
 * Usage:
 *  Pushbutton on backside:
 *    Initiate calibration of SCD30 CO2 sensor.
 *  RST Pushbutton on backside:
 *    Restart the device, especially when SD card was removed or inserted.
 *  ON/OFF slide switch on backside:
 *    Turn the device (including display backlight) on and off.
 * 
 * Note:
 *  If used with a Adafruit Adalogger FeatherWing make sure to resolder
 *  CS pin for TFT display on the display breakout board and modify pin
 *  definitions accordingly, as the presoldered CS pin is 10 which is also
 *  the CS pin for the SD card on the Adalogger FeatherWing.
 * 
 * created        14.04.2021
 * last modified  29.05.2021
 * by             Jannik Sehringer
 * for            Laboratory for Sensors,
 *                IMTEK - Department of Microsystems Engineering,
 *                University of Freiburg
 * 
******************************************************************************/

/* Include needed libraries */
#include <SPI.h>
#include <SD.h>
#include <Wire.h>                             // I2C
#include <RTClib.h>                           // Real time clock
#include <SparkFun_SCD30_Arduino_Library.h>   // CO2 Sensor
#include <Adafruit_SleepyDog.h>               // Watchdog timer
#include <Adafruit_GFX.h>                     // Graphics
#include <Adafruit_HX8357.h>                  // 3.5" TFT display
#include "Graphics.h"                         // draw graphic elements
#include "bmpDraw.h"                          // draw bitmap files

/* Define pin names */
// SPI
#define SD_CS   5   // on Adafruit 3.5" 480x320 TFT Feahterwing
#define SD2_CS  4   // on Feather M0 uSD Adalogger
#define TFT_CS  9
#define TFT_DC  10  // Data/Command pin of TFT display
#define TFT_RST -1  // RST can be set to -1 if you tie it to Arduino's reset
// Buttons
#define CALIB   14  // button to init calibration

/* Colors used on display in 16 bit 565-RGB (5 red, 6 green, 5 blue) */
// convert 3 8 bit component RGB color to 16 bit 565-RGB color
#define RGB_TO_565RGB(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
// color definitions composed of: red   green blue
#define WHITE       RGB_TO_565RGB(0xFF, 0xFF, 0xFF)
#define GREY        RGB_TO_565RGB(0x7F, 0x7F, 0x7F)
#define BLACK       RGB_TO_565RGB(0x00, 0x00, 0x00)
#define RED         RGB_TO_565RGB(0xFF, 0x00, 0x00)
#define ORANGE      RGB_TO_565RGB(0xFF, 0x7F, 0x00)
#define YELLOW      RGB_TO_565RGB(0xB8, 0xB8, 0x00)
#define GREEN       RGB_TO_565RGB(0x00, 0xC0, 0x00)
#define BLUE        RGB_TO_565RGB(0x00, 0x00, 0xFF)
// colors of IMTEK logo
#define IMTEK_BLUE  RGB_TO_565RGB(0x18, 0x10, 0x77)
#define IMTEK_RED   RGB_TO_565RGB(0xBA, 0x24, 0x26)
// frequently used colors
#define BACKGROUND_COLOR  BLACK
#define TEXT_COLOR        WHITE

/* Global constants */
// directories , files and contents
// handable filenames must be 8.3 format -> 13 chars incl. trailing 0
#define DIRECTORY         "Data"
#define DEFAULT_FILE_NAME "datalogg.csv"
#define FILE_HEADER   	  "dateTime, co2, temp, rh"
#define IMTEK_LOGO_SMALL  "g100x44.bmp"
#define IMTEK_LOGO_BIG    "w460x203.bmp"
// calibration
#define BACKGROUND_CO2    417   ///< ppm value of atmospheric background CO2
#define CALIBRATION_TIME  300   ///< seconds to wait before calibration

/* Instances of used sensors and peripherals */
SCD30 scd30;
RTC_DS3231 rtc;
Adafruit_HX8357 tft(TFT_CS, TFT_DC, TFT_RST);

/*****************************************************************************
    setup - initializations
*****************************************************************************/
void setup() {
  /* Activate peripherals */
  Wire.begin();
  tft.begin();
  scd30.begin();
  rtc.begin();
  // if SD card on display shield is not found try SD card on Adalogger
  if (!SD.begin(SD_CS)) {
    SD.begin(SD2_CS);
  }
  Graphics::useDisplay(&tft);   // pass display to all classes that print on it
  
  /* initialize peripherals */
  // TFT display
  tft.cp437(true);
  tft.setRotation(1);
  // during start up clear screen to white print headline and logo
  tft.fillScreen(WHITE);
  uint8_t size = 4;
  int16_t center = (tft.width() - (10*size*CHAR_W - CHAR_W)) / 2;
  Label startup(center, 20, "CO2FreiMon", size, IMTEK_BLUE, 3);
  center = (tft.height() - 203 + size*CHAR_H + 20) / 2;
  bmpReader::draw(IMTEK_LOGO_BIG, 10, center);

  // CO2 sensor
  scd30.setAutoSelfCalibration(false);    // deactivate auto calibration
  scd30.setAltitudeCompensation(278);     // Freiburg is 278 m above sea level
  scd30.setTemperatureOffset(0);          // no temperature offset

  // Watchdog
  Watchdog.enable(8000);  // set watchdog interval 8 s

  // SD
  if (!SD.exists(DIRECTORY)) {  // if it does not exist yet
    SD.mkdir(DIRECTORY);        // create directory for data files
  }

  /* Pin modes */
  pinMode(CALIB, INPUT_PULLUP);

  // wait 3 seconds to show startup logo, then turn display black
  delay(3000);
  tft.fillScreen(BACKGROUND_COLOR);
}

/*****************************************************************************    
    loop - code to be run continiously
*****************************************************************************/
void loop() {
  // filename of the datafile
  static String datafile;

  // store last second, minute and day to trigger action on change
  // init with values that do not occur naturally to trigger action on startup
  static uint8_t lastDay = 0;
  static uint8_t lastMinute = 60;
  static uint8_t lastSecond = 60;

  // store calibration status
  static bool calibrationPending = false;

  // graphical elements on the screen
  static HeaderBar hbar(46, GREY, TEXT_COLOR, IMTEK_LOGO_SMALL);
  static ValueBar vbarCO2(20, 53, 440, 80, IMTEK_BLUE, TEXT_COLOR, "CO2", "ppm", 3);
  static ValueBar vbarTemp(20, 142, 440, 80, IMTEK_BLUE, TEXT_COLOR, "Temp", "°C");
  static ValueBar vbarRH(20, 231, 440, 80, IMTEK_BLUE, TEXT_COLOR, "RH", "%");
  // Though not visible most of the time warning must be in scope of loop
  static CalibrationWarning
    calibWarning(20, hbar.height()+10, 440, tft.height()-hbar.height()-20,
                 IMTEK_RED, TEXT_COLOR);

  Watchdog.reset();   // keep watchdog happy
  
  DateTime newTime = rtc.now();   // get time of this loops execution

  // if time has changed update it on display
  if (newTime.minute() != lastMinute) {
    lastMinute = newTime.minute();
    hbar.updateTime(newTime);

    // when date has changed update it on display and start a new data file
    if (newTime.day() != lastDay) {
      lastDay = newTime.day();
      hbar.updateDate(newTime);

      // get new file name. As this is also called on startup
      // only write file header if file did not exist yet
      datafile = getFilename();
      if (!SD.exists(datafile)) {
        printSD(datafile, FILE_HEADER);
      }
    }   // day changed
  }   // minute changed

  // if calibration status is pending and second has changed refresh
  // countdown until calibration
  if (calibrationPending && (newTime.second() != lastSecond)) {
    lastSecond = newTime.second();
    calibWarning.refreshCountdown(newTime);

    // if calibration status is pending and calibration time is reached
    // calibrate the CO2 sensor, log it in output file and refresh
    // the display to show the value readouts again
    if (newTime >= calibWarning.getCalibrationTime()) {
      scd30.setForcedRecalibrationFactor(BACKGROUND_CO2);
      calibrationPending = false;

      File file = SD.open(datafile, FILE_WRITE);
      if (file) {
        file.printf(
          "# Calibration\n"
          "# Setting last CO2 value to background value of %d ppm.\n",
          BACKGROUND_CO2
        );
        file.close();
      }

      // remove calibration warning and reprint value bars
      calibWarning.erase(BACKGROUND_COLOR);
      vbarCO2.draw();
      vbarTemp.draw();
      vbarRH.draw();
    }
  }   // calibration pending

  // if sensor has measured new values
  if (scd30.dataAvailable()) {
    // get measurement data
    uint16_t co2  = scd30.getCO2();
    float    temp = scd30.getTemperature();
    float    rh   = scd30.getHumidity();

    // Open file and write the data to it
    File file = SD.open(datafile, FILE_WRITE);
    if (file) {
      // if RTC is running write date and time to file
      if (!rtc.lostPower()) {
        DateTime now = rtc.now();
        file.printf(
          "%i/%02i/%02i %02i:%02i:%02i",
          now.year(), now.month(), now.day(),
          now.hour(), now.minute(), now.second()
        );
      }

      // write measurement data to file and close it afterwards
      file.printf(", %i, %.2f, %.2f\n", co2, temp, rh);
      file.close();
    }

    // update values on display
    if (!calibrationPending) {
      // change color according to warning level
           if (co2 <  400) vbarCO2.changeColor(GREY);
      else if (co2 < 1000) vbarCO2.changeColor(GREEN);
      else if (co2 < 1500) vbarCO2.changeColor(YELLOW);
      else if (co2 < 2000) vbarCO2.changeColor(ORANGE);
      else if (co2 > 2000) vbarCO2.changeColor(IMTEK_RED);

      // update values in value bars...
      vbarCO2.refreshValue(co2);
      vbarTemp.refreshValue(temp);
      vbarRH.refreshValue(rh);
    } else {
      // or in calibration warning if calibration is pending
      calibWarning.refreshCO2(co2);
    }
  }   // data available

  // if button is pressed and calibration is not already initiated
  // start calibration sequence
  if (!digitalRead(CALIB) && !calibrationPending) {
    // set calibration status on pending
    calibrationPending = true;

    // clear display and print calibration information
    vbarCO2.erase(BACKGROUND_COLOR);
    vbarTemp.erase(BACKGROUND_COLOR);
    vbarRH.erase(BACKGROUND_COLOR);
    calibWarning.setCalibrationTime(rtc.now() + TimeSpan(CALIBRATION_TIME));
    calibWarning.print();
  }
}

/*****************************************************************************    
    Functions - self implemented functions
*****************************************************************************/

// create filename consisting of the date, including directory
String getFilename(void) {
  String filename = DEFAULT_FILE_NAME;
  if (!rtc.lostPower()) {
    DateTime currentTime = rtc.now();
    filename[0] = '0' + (currentTime.year() / 10) % 10;
    filename[1] = '0' + currentTime.year() % 10;
    filename[2] = '-';
    filename[3] = '0' + currentTime.month() / 10;
    filename[4] = '0' + currentTime.month() % 10;
    filename[5] = '-';
    filename[6] = '0' + currentTime.day() / 10;
    filename[7] = '0' + currentTime.day() % 10;
  }
  return String(DIRECTORY) + '/' + filename;
}


// print given text into given file on SD card
void printSD(String filename, const char* text) {
  File file = SD.open(filename, FILE_WRITE);
  if (file) {
    file.println(text);
    file.close();
  }
}