/*
  Test TCLite on Arduino Due

  Mario Ban, 12.2014, 01.2015

  todo:
    - count and display at fix column the # of terminal-state events, registration-state events, ...
    - use tft diaplay for log (with colors) ok
    - register for all data callbacks, display data (hex/text: first 20 chars)

  GNU General Public License v2.0
  Copyright (C) 2015 Mario Ban
*/

#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "InputDebounce.h"

/* TFT: see also port_TCL_Logger */
#define TCL_LOG_TFT_ADA_ILI9340   0   /* TFT 2.2"; 1: enable, 0: disable */
#define TCL_LOG_TFT_ADA_HX8357    1   /* TFT 3.5"; 1: enable, 0: disable */

#if TCL_LOG_TFT_ADA_ILI9340 > 0
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

#if TCL_LOG_TFT_ADA_HX8357 > 0
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"
// Arduino Due hardware SPI pins
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8 // RST can be set to -1 if you tie it to Arduino's reset
#endif

#include "TCLite.h"

static LiquidCrystal_I2C s_lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 20 chars and 4 line display

#if TCL_LOG_TFT_ADA_ILI9340 > 0
// Using software SPI is really not suggested, its incredibly slow
//Adafruit_ILI9340 s_tft = Adafruit_ILI9340(_cs, _dc, _mosi, _sclk, _rst, _miso);
// Use hardware SPI
Adafruit_ILI9340 s_tft = Adafruit_ILI9340(_cs, _dc, _rst); // Arduino Uno: MOSI 11, SCLK 13
#endif

#if TCL_LOG_TFT_ADA_HX8357 > 0
// Use hardware SPI
Adafruit_HX8357 s_tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST); // Arduino Uno: #13, #12, #11 and the above for CS/DC
#endif

#define APPLICATION_ID         100
#define LOG_HEX_DUMP_COLUMNS   32

#define LCD_TERMINAL_STATE_COL        0   /* 20 chars */
#define LCD_TERMINAL_STATE_ROW        2
#define LCD_REGISTRATION_STATE_COL    0   /* 20 chars */
#define LCD_REGISTRATION_STATE_ROW    3
#define LCD_TCLITE_BUSY_COL          18   /* 1 char */
#define LCD_TCLITE_BUSY_ROW           0
#define LCD_TCLITE_SEND_COL          17   /* 1 char */
#define LCD_TCLITE_SEND_ROW           0
#define LCD_TCLITE_RECV_COL          16   /* 1 char */
#define LCD_TCLITE_RECV_ROW           0
#define LCD_TCLITE_DATA_RECV_COL      0   /* 20 chars */
#define LCD_TCLITE_DATA_RECV_ROW      1
#define LCD_ACTIVITY_COL             19   /* 1 char */
#define LCD_ACTIVITY_ROW              0

#define BUTTON_PIN_SEND_DATA_ACK       22
#define BUTTON_PIN_SEND_DATA_NOT_ACK   23
#define BUTTON_DEBOUNCE_DELAY          20   // [ms], do tests with e.g. 300

#define TCL_DATA_ACK_SEND_BACK   1   /* 1: enable, 0: disable */
#define TCL_DATA_ACK_SEND_CASE   4   /* send-data-ack test-case: 1..5 */

#define TCL_DATA_NOT_ACK_SEND_BACK   1   /* 1: enable, 0: disable */
#define TCL_DATA_NOT_ACK_SEND_CASE   4   /* send-data-not-ack test-case: 1..5 */

static TCL_Error s_error;
static TCL_UInt32 s_processingInterval; /* [ms] */

static TCL_Bool s_connected = TCL_FALSE;
static TCL_TerminalRegistrationStateType s_registrationState = TCL_TERMINAL_REGISTRATION_STATE_NOT_REGISTERED;

static TCL_Bool s_busy_1 = TCL_FALSE;   /* data-acknowledged */
static TCL_Bool s_busy_2 = TCL_FALSE;   /* data-not-acknowledged */
static TCL_Bool s_busy = TCL_FALSE;

static TCL_ReqSendDataAck s_reqSendDataAck;
static TCL_ReqSendDataNotAck s_reqSendDataNotAck;

static TCL_RequestId s_requestId = 1000;

static TCL_Bool s_send_1 = TCL_FALSE;   /* data-ack */
static TCL_SourceAddress s_sourceAddress_1;

static TCL_Bool s_send_2 = TCL_FALSE;   /* data-not-ack */
static TCL_SourceAddress s_sourceAddress_2;

static TCL_String s_rfsi; /* test: self RFSI */
static TCL_Char s_rfsiBuffer[TCL_ADDRESS_RFSI_STRING_BUFFER_SIZE];

class ActivityLED
{
public:
  ActivityLED();
  void setup(unsigned char pin, unsigned long interval);
  void process(unsigned long now);
private:
  // implicitly implemented, not to be used
  ActivityLED(const ActivityLED&);
  ActivityLED& operator=(const ActivityLED&);
  //
  unsigned char _pin;
  unsigned long _interval;
  unsigned long _timeStamp;
  bool _toggle;
  bool _enabled;
};

class ActivityLCD
{
public:
  ActivityLCD();
  void setup(LiquidCrystal_I2C* lcd, unsigned char col, unsigned char row, unsigned long interval);
  void process(unsigned long now);
private:
  // implicitly implemented, not to be used
  ActivityLCD(const ActivityLCD&);
  ActivityLCD& operator=(const ActivityLCD&);
  //
  LiquidCrystal_I2C* _lcd;
  unsigned char _col;
  unsigned char _row;
  unsigned long _interval;
  unsigned long _timeStamp;
  unsigned char _count;
  bool _enabled;
};

class IndicationLCD
{
public:
  IndicationLCD();
  void setup(LiquidCrystal_I2C* lcd, unsigned char col, unsigned char row, char indChar, unsigned long indDelay);
  void show(unsigned long now);
  void show();
  void hide();
  void process(unsigned long now);
private:
  // implicitly implemented, not to be used
  IndicationLCD(const IndicationLCD&);
  IndicationLCD& operator=(const IndicationLCD&);
  //
  LiquidCrystal_I2C* _lcd;
  unsigned char _col;
  unsigned char _row;
  char _indChar;
  unsigned long _indDelay;
  unsigned long _timeStamp;
  bool _on;
  bool _doShow;
  bool _enabled;
};

static ActivityLED s_activityLED;
static ActivityLCD s_activityLCD;
static IndicationLCD s_indicationLCD_send;
static IndicationLCD s_indicationLCD_recv;
static InputDebounce s_buttonSendDataAck;
static InputDebounce s_buttonSendDataNotAck;

void setup()
{
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
  s_lcd.init(); // initialize the lcd
  s_lcd.backlight();
  s_lcd.print(F("Test TCLite"));
  s_lcd.setCursor(0, 1);
  s_lcd.print("v" TCL_VERSION " (" TCL_BUILD ")");
  
  updateLCD_TerminalState(TCL_FALSE, "");
  updateLCD_RegistrationState(TCL_TERMINAL_REGISTRATION_STATE_NOT_REGISTERED, 0, 0, 0, TCL_SYSTEM_MODE_NORMAL);
  updateLCD_Busy();
  
  //updateLCD_TerminalState(TCL_TRUE, "994110901");
  //updateLCD_RegistrationState(TCL_TERMINAL_REGISTRATION_STATE_REGISTERED, 994, 1, 2, TCL_SYSTEM_MODE_NORMAL);
  
  // TFT Adafruit ILI9340
#if TCL_LOG_TFT_ADA_ILI9340 > 0
  s_tft.begin();
  s_tft.setRotation(1);
  s_tft.fillScreen(ILI9340_BLACK);
  s_tft.setCursor(0, 0);
  s_tft.setTextColor(ILI9340_GREEN);
  s_tft.setTextSize(2);
  s_tft.println(F("Test TCLite"));
  s_tft.println("v" TCL_VERSION " (" TCL_BUILD ")");
  s_tft.println("-------------------------");
  s_tft.setTextColor(ILI9340_WHITE);
  s_tft.setTextSize(1);
#endif
  
  // TFT Adafruit HX8357
#if TCL_LOG_TFT_ADA_HX8357 > 0
  s_tft.begin(HX8357D);
  s_tft.setRotation(1);
  s_tft.fillScreen(HX8357_BLACK);
  s_tft.setCursor(0, 0);
  s_tft.setTextColor(HX8357_GREEN);
  s_tft.setTextSize(2);
  s_tft.println(F("Test TCLite"));
  s_tft.println("v" TCL_VERSION " (" TCL_BUILD ")");
  s_tft.println("-------------------------");
  s_tft.setTextColor(HX8357_WHITE);
  s_tft.setTextSize(1);
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
  s_activityLED.setup(LED_BUILTIN, 500); // 500ms on, 500ms off
  
  // activity LCD
  s_activityLCD.setup(&s_lcd, LCD_ACTIVITY_COL, LCD_ACTIVITY_ROW, 1000); // 1s interval (cycle through 4 chars)
  s_indicationLCD_send.setup(&s_lcd, LCD_TCLITE_SEND_COL, LCD_TCLITE_SEND_ROW, (char)0x7e /* right arrow */, 300 * 1000); // 5min delay (not used)
  s_indicationLCD_recv.setup(&s_lcd, LCD_TCLITE_RECV_COL, LCD_TCLITE_RECV_ROW, (char)0x7f /* left arrow */, 300); // 300ms delay
  
  // buttons
  s_buttonSendDataAck.setup(BUTTON_PIN_SEND_DATA_ACK, BUTTON_DEBOUNCE_DELAY);
  s_buttonSendDataNotAck.setup(BUTTON_PIN_SEND_DATA_NOT_ACK, BUTTON_DEBOUNCE_DELAY);
  
  /* TCLite */
  setup_TCLite();
}

void loop()
{
  // put your main code here, to run repeatedly:
  
  static unsigned int buttonStateOnCount_SendDataAck = 0;
  static unsigned int buttonStateOnCount_SendDataNotAck = 0;
  
  /* TCLite */
  loop_TCLite();
  
  // todo: check if write buffer remaining (TTCI_SL PropagateWriteToListeners)
  if(Serial1 && Serial1.available()) {
    delay(1); // [ms]
  }
  else {
    delay(s_processingInterval);
  }
  
  unsigned long now = millis();
  
  // buttons: poll button state, return on-time [ms] if pressed (debounced)
  unsigned int buttonOnTime_SendDataAck = s_buttonSendDataAck.process(now);
  unsigned int buttonOnTime_SendDataNotAck = s_buttonSendDataNotAck.process(now);
  
  if(buttonOnTime_SendDataAck) {
    unsigned int count = s_buttonSendDataAck.getStateOnCount();
    if(buttonStateOnCount_SendDataAck != count) {
      buttonStateOnCount_SendDataAck = count;
      // test service: send-data-acknowledged
      s_send_1 = TCL_TRUE;
      TCL_LogInfo("Button TCL_ReqSendDataAck pressed");
    }
    else {
      //TCL_LogInfo("Button TCL_ReqSendDataAck still pressed"); // change this; log count, time
    }
  }
  if(buttonOnTime_SendDataNotAck) {
    unsigned int count = s_buttonSendDataNotAck.getStateOnCount();
    if(buttonStateOnCount_SendDataNotAck != count) {
      buttonStateOnCount_SendDataNotAck = count;
      // test service: send-data-not-acknowledged
      s_send_2 = TCL_TRUE;
      TCL_LogInfo("Button TCL_ReqSendDataNotAck pressed");
    }
    else {
      //TCL_LogInfo("Button TCL_ReqSendDataNotAck still pressed"); // change this; log count, time
    }
  }
  
  // activity LED
  s_activityLED.process(now);
  
  // activity LCD
  s_activityLCD.process(now);
  s_indicationLCD_send.process(now);
  s_indicationLCD_recv.process(now);
}
