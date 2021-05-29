# CO<sub>2</sub>FreiMon - Freiburg's CO<sub>2</sub> Monitor 


Contributors: Jannik Sehringer (major), Jochen Kieninger (minor)

Contact: Dr. Jochen Kieninger, kieninger@imtek.uni-freiburg.de

[Laboratory for Sensors](https://www.imtek.de/laboratories/sensors/sensors_home?set_language=en), IMTEK, University of Freiburg

<img src="CO2Monitor.jpg" width="400">

## About
This repo contains code to operate a Sensirion SCD30 CO<sub>2</sub> sensor connected to an Adafruit Feather M0 and a Adafruit 3.5" TFT FeatherWing display with the abillity to store data on a SD card. It was created as part of a bachelor thesis on air qualitiy sensors for COVID-19 prevention supported by the Department of Occupational Health and Safety at the University of Freiburg.

## Hardware
The following components are needed:
* Adafruit Feather M0 - with stacking headers
* Adafruit 3.5" 480x320 TFT FeatherWing (including SD card)
* Adafruit DS3231 Precision Real Time Clock Featherwing (including backup battery) - with donwards pointing male headers
* Sensirion SCD30 - CO<sub>2</sub> Sensor
* Pushbutton

To assamble the deviece:
1. Solder the SCD30 pins to the pins of the Feather M0 board as follows:
    - VIN - 3V3
    - GND - GND
    - SCL - SCL
    - SDA - SDA
2. Solder the pushbutton between pin A0 (14) and GND of the Feather board
3. Stack the Feaher boards on top of each other on the backside of the display board

## Library Requirements
The following libraries are needed.

These should be included in the installation of the Arduino IDE:
* SPi.h
* SD.h
* Wire.h

These are donlowdable via the Arduino library manager:
* RTClib.h
* SparkFun_SCD30_Arduino_Library.h
* Adafruit_SleepyDog.h
* Adafruit_GFX.h
* Adafruit_HX8357.h



## How to use the code

