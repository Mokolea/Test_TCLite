# Test TCLite
Test usage of library TCLite on an [Arduino Due](http://www.arduino.cc/en/Main/ArduinoBoardDue) board

Terminal Controller Lite - control a TETRAPOL radio terminal (send/receive data)

Current TCLite: *Version 1.2.0 2015-04-07*

## Build
 - Open in Arduino IDE: Test_TCLite.ino
 - Target: Arduino Due (programming port)

## Dependencies (libraries)
 - TCLite
 - LiquidCrystal_I2C
 - Adafruit_GFX
 - Adafruit_HX8357

## Hardware setup
Schematic (Eagle), pic, ...

## Parts list
 - [Arduino Due](http://www.arduino.cc/en/Main/ArduinoBoardDue) board,
   [Getting started](http://www.arduino.cc/en/Guide/ArduinoDue) guide
 - Serial <-> TTL D-Sub 9 M MAX232 5V
 - Level shifter 3.3V <-> 5V, [Adafruit 757](http://www.adafruit.com/product/757)
 - LCD 20x4 I2C 5V
 - TFT 3.5" SPI 3.3V, [Adafruit 2050](http://www.adafruit.com/product/2050)
 - I/O Buttons LEDs ...
 - FTDI serial TTL-232 USB cable, [Adafruit 70](http://www.adafruit.com/product/70)
 - ...

## Test equipment
 - TETRAPOL Data Simulator, serial null-modem cable
 - TETRAPOL radio terminal TPH 700, serial data adapter cable

-- Mario
