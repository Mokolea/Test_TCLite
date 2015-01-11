/*
  Test TCLite on Arduino Due
  
  Mario Ban, 12.2014, 01.2015
  
  todo:
    - count and display at fix column the # of terminal-state events, registration-state events, ...
    - use tft diaplay for log (with colors) ok
    - register for all data callbacks, display data (hex/text: first 20 chars)
  
*/

#include <Wire.h>
#include "LiquidCrystal_I2C.h"

#define TCL_LOG_TFT   1   /* 1: enable, 0: disable */

#if TCL_LOG_TFT > 0
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"
// SPI
//#define _sclk 13
//#define _miso 12
//#define _mosi 11
// Arduino Due hardware SPI pins
#define _cs 10
#define _dc 9
#define _rst 8
#endif

#include "TCLite.h"

static LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 20 chars and 4 line display

#if TCL_LOG_TFT > 0
// Using software SPI is really not suggested, its incredibly slow
//Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _mosi, _sclk, _rst, _miso);
// Use hardware SPI
Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst); // Arduino Uno: MOSI 11, SCLK 13
#endif

#define APPLICATION_ID         100
#define LOG_HEX_DUMP_COLUMNS   32

#define LCD_TERMINAL_STATE_COL        0   /* 20 chars */
#define LCD_TERMINAL_STATE_ROW        2
#define LCD_REGISTRATION_STATE_COL    0   /* 20 chars */
#define LCD_REGISTRATION_STATE_ROW    3
#define LCD_TCLITE_BUSY_COL          18   /* 1 char */
#define LCD_TCLITE_BUSY_ROW           0
#define LCD_ACTIVITY_COL             19   /* 1 char */
#define LCD_ACTIVITY_ROW              0

static TCL_Error s_error;
static TCL_UInt32 s_processingInterval; /* [ms] */

static TCL_Bool s_connected = TCL_FALSE;
static TCL_TerminalRegistrationStateType s_registrationState = TCL_TERMINAL_REGISTRATION_STATE_NOT_REGISTERED;

class ActivityLED
{
public:
  ActivityLED()
   : _pin(LED_BUILTIN)
   , _interval(1000)
   , _last(0)
   , _toggle(false)
   , _enabled(false)
  {}
  void setup(unsigned char pin, unsigned long interval) {
    if(interval >= 100) {
      _pin = pin;
      _interval = interval;
      pinMode(_pin, OUTPUT);
      _enabled = true;
    }
    else {
      _enabled = false;
    }
  }
  void process(unsigned long now) {
    if(!_enabled) return;
    if(now > _last + _interval) {
      _last = now;
      if(_toggle) {
        digitalWrite(_pin, HIGH);
        //TCL_LogInfo("ActivityLED HIGH");
      }
      else {
        digitalWrite(_pin, LOW);
        //TCL_LogInfo("ActivityLED LOW");
      }
      _toggle = !_toggle;
    }
  }
private:
  unsigned char _pin;
  unsigned long _interval;
  unsigned long _last;
  bool _toggle;
  bool _enabled;
};

class ActivityLCD
{
public:
  ActivityLCD()
   : _lcd(0)
   , _col(0)
   , _row(0)
   , _interval(1000)
   , _last(0)
   , _count(0)
   , _enabled(false)
  {}
  void setup(LiquidCrystal_I2C *lcd, unsigned char col, unsigned char row, unsigned long interval) {
    if(interval >= 100) {
      _lcd = lcd;
      _col = col;
      _row = row;
      _interval = interval;
      _enabled = true;
    }
    else {
      _enabled = false;
    }
  }
  void process(unsigned long now) {
    if(!_enabled) return;
    if(now > _last + _interval) {
      _last = now;
      _lcd->setCursor(_col, _row);
      if((_count+3) % 4 == 0) {
        _lcd->print(" ");
      }
      else if((_count+2) % 4 == 0) {
        _lcd->print((char)0x7e); /* right arrow */
      }
      else if((_count+1) % 4 == 0) {
        _lcd->print(" ");
      }
      else if(_count % 4 == 0) {
        _lcd->print((char)0x7f); /* left arrow */
      }
      else {
        _lcd->print("?"); /* never */
      }
      _count++;
    }
  }
private:
  LiquidCrystal_I2C *_lcd;
  unsigned char _col;
  unsigned char _row;
  unsigned long _interval;
  unsigned long _last;
  unsigned char _count;
  bool _enabled;
};

static ActivityLED activityLED;
static ActivityLCD activityLCD;

void setup() {
  // put your setup code here, to run once:
  
  // Logger
  Serial.begin(9600); // open the serial port at 9600 bps: port_TCL_Logger
  while(!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Serial: Initialized 9600");
  
  Serial2.begin(115200); // open the serial port at 115200 bps: port_TCL_Logger
  while(!Serial2) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial2.println("Serial2: Initialized 115200");
  
  // LCD
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.print("Test TCLite");
  lcd.setCursor(0, 1);
  lcd.print("v" TCL_VERSION " (" TCL_BUILD ")");
  
  updateLCD_TerminalState(TCL_FALSE, "");
  updateLCD_RegistrationState(TCL_TERMINAL_REGISTRATION_STATE_NOT_REGISTERED, 0, 0, 0);
  
  /*updateLCD_TerminalState(TCL_TRUE, "994110901");
  updateLCD_RegistrationState(TCL_TERMINAL_REGISTRATION_STATE_REGISTERED, 994, 1, 2);*/
  
  // TFT
#if TCL_LOG_TFT > 0
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9340_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9340_GREEN);
  tft.setTextSize(2);
  tft.println("Test TCLite");
  tft.println("v" TCL_VERSION " (" TCL_BUILD ")");
  tft.println("-------------------------");
  tft.setTextColor(ILI9340_WHITE);
  tft.setTextSize(1);
#endif
  
  TCL_LogInfo("+++ test TCL_Logger");
  TCL_LogInfo("TCL_LOG_INFO");
  TCL_LogFatal("TCL_LOG_FATAL");
  TCL_LogError("TCL_LOG_ERROR");
  TCL_LogWarning("TCL_LOG_WARNING");
  TCL_LogDebug("TCL_LOG_DEBUG");
  TCL_LogFunction("TCL_LOG_FUNCTION");
  TCL_LogInfo("--- test TCL_Logger");
  
  // activity LED
  activityLED.setup(LED_BUILTIN, 500); // pin out 13; 500ms on, 500ms off
  
  // activity LCD
  activityLCD.setup(&lcd, LCD_ACTIVITY_COL, LCD_ACTIVITY_ROW, 1000); // col 20, row 1; 500ms interval
  
  /* TCLite */
  
  /* initialize */
  
  TTCI_SL_PORT_NUMBER = 1;
  TTCI_SL_PORT_PARAM = TCL_SERIAL_PORT_PARAM_9600_8N1;
  
  TCL_ErrorConstruct(&s_error);
  
  s_processingInterval = TCL_Initialize(&s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_Initialize failed");
    exit(EXIT_FAILURE);
  }
  
  /* register callbacks */
  
  TCL_EvtTerminalStateRegisterCallback(TCL_EvtTerminalStateCallback, &s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_EvtTerminalStateRegisterCallback failed");
    exit(EXIT_FAILURE);
  }
  
  TCL_EvtRegistrationStateRegisterCallback(TCL_EvtRegistrationStateCallback, &s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_EvtRegistrationStateRegisterCallback failed");
    exit(EXIT_FAILURE);
  }
  
  /* start */
  
  TCL_Start(APPLICATION_ID, &s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_Start failed");
    exit(EXIT_FAILURE);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  /* TCLite */
  
  /* processing */
  
  TCL_Process(&s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_Process failed");
    exit(EXIT_FAILURE);
  }
  
  delay(s_processingInterval);

  unsigned long now = millis();
  
  // activity LED
  activityLED.process(now);
  
  // activity LCD
  activityLCD.process(now);
}

