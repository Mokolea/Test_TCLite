# Test TCLite
Test usage of library TCLite on an [Arduino Due](http://www.arduino.cc/en/Main/ArduinoBoardDue) board

Terminal Controller Lite - control a TETRAPOL radio terminal to handle data transmission (send/receive)

Current TCLite: *Version 1.2.0 2015-04-07*

[![Release](https://img.shields.io/github/release/Mokolea/Test_TCLite.svg)](https://github.com/Mokolea/Test_TCLite/releases)
![License](https://img.shields.io/github/license/Mokolea/Test_TCLite.svg)

## Build
 - Open in Arduino IDE (currently using v1.6.x, Mac OS X): Test_TCLite.ino
 - Select target: Arduino Due (programming port)

## Dependencies (libraries)
 - TCLite, (c) Atos AG Switzerland
 - Arduino IDE Library Manager:
   - LiquidCrystal_I2C
   - Adafruit GFX Library
   - Adafruit HX8357 Library
   - InputDebounce

## Hardware setup
**TODO...**
Schematic (Eagle), PCB (Fritzing), pic, ...

## Parts list
 - [Arduino Due](http://www.arduino.cc/en/Main/ArduinoBoardDue) board,
   see [getting started](http://www.arduino.cc/en/Guide/ArduinoDue) guide
 - Serial <-> TTL D-Sub 9 M MAX232 5V
 - Level shifter 3.3V <-> 5V, [Adafruit 757](http://www.adafruit.com/product/757)
 - LCD 20x4 I2C 5V
 - TFT 3.5" SPI 3.3V, [Adafruit 2050](http://www.adafruit.com/product/2050)
 - I/O Buttons LEDs ...
 - FTDI serial TTL-232 USB cable, [Adafruit 70](http://www.adafruit.com/product/70)
 - ...

## Test equipment
 - TETRAPOL data simulator, serial null-modem cable
 - TETRAPOL radio terminal TPH 700, serial data adapter cable

*GNU General Public License v2.0*

-- Mario
