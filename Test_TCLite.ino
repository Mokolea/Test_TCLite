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

/* TFT: see also TCL_Logger.c */
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

static LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 20 chars and 4 line display

#if TCL_LOG_TFT_ADA_ILI9340 > 0
// Using software SPI is really not suggested, its incredibly slow
//Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _mosi, _sclk, _rst, _miso);
// Use hardware SPI
Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst); // Arduino Uno: MOSI 11, SCLK 13
#endif

#if TCL_LOG_TFT_ADA_HX8357 > 0
// Use hardware SPI
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST); // Arduino Uno: #13, #12, #11 and the above for CS/DC
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
#define BUTTON_DEBOUNCE_DELAY         100   /* [ms] */

#define TCL_DATA_ACK_SEND_BACK   1   /* 1: enable, 0: disable */
#define TCL_DATA_ACK_SEND_CASE   4   /* send-data-ack test-case: 1..5 */

#define TCL_DATA_NOT_ACK_SEND_BACK   1   /* 1: enable, 0: disable */
#define TCL_DATA_NOT_ACK_SEND_CASE   4   /* send-data-not-ack test-case: 1..5 */

static TCL_Error s_error;
static TCL_UInt32 s_processingInterval; /* [ms] */

static TCL_Bool s_connected = TCL_FALSE;
static TCL_TerminalRegistrationStateType s_registrationState = TCL_TERMINAL_REGISTRATION_STATE_NOT_REGISTERED;

static TCL_Bool s_busy_1 = TCL_FALSE;   /* data-ack */
static TCL_Bool s_busy_2 = TCL_FALSE;   /* data-not-ack */
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
  ActivityLED()
    : _pin(LED_BUILTIN)
    , _interval(1000)
    , _timeStamp(0)
    , _toggle(false)
    , _enabled(false)
  {
  }
  void setup(unsigned char pin, unsigned long interval)
  {
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
  void process(unsigned long now)
  {
    if(!_enabled) {
      return;
    }
    if(now - _timeStamp > _interval) {
      _timeStamp = now;
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
  unsigned long _timeStamp;
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
    , _timeStamp(0)
    , _count(0)
    , _enabled(false)
  {
  }
  void setup(LiquidCrystal_I2C* lcd, unsigned char col, unsigned char row, unsigned long interval)
  {
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
  void process(unsigned long now)
  {
    if(!_enabled) {
      return;
    }
    if(now - _timeStamp > _interval) {
      _timeStamp = now;
      _lcd->setCursor(_col, _row);
      if((_count + 3) % 4 == 0) {
        _lcd->print(" ");
      }
      else if((_count + 2) % 4 == 0) {
        _lcd->print("x");
      }
      else if((_count + 1) % 4 == 0) {
        _lcd->print(" ");
      }
      else if(_count % 4 == 0) {
        _lcd->print("o");
      }
      else {
        _lcd->print("?"); /* never */
      }
      _count++;
    }
  }
private:
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
  IndicationLCD()
    : _lcd(0)
    , _col(0)
    , _row(0)
    , _indChar('I')
    , _indDelay(300)
    , _timeStamp(0)
    , _on(false)
    , _doShow(false)
    , _enabled(false)
  {
  }
  void setup(LiquidCrystal_I2C* lcd, unsigned char col, unsigned char row, char indChar, unsigned long indDelay)
  {
    if(indDelay >= 100) {
      _lcd = lcd;
      _col = col;
      _row = row;
      _indChar = indChar;
      _indDelay = indDelay;
      _enabled = true;
    }
    else {
      _enabled = false;
    }
  }
  void show(unsigned long now)
  {
    if(!_enabled) {
      return;
    }
    _timeStamp = now;
    if(!_on) {
      _on = true;
      _lcd->setCursor(_col, _row);
      _lcd->print(_indChar);
    }
  }
  void show()
  {
    if(!_enabled) {
      return;
    }
    _doShow = true; // wait till next process call
  }
  void hide()
  {
    if(!_enabled) {
      return;
    }
    _doShow = false;
    _timeStamp -= _indDelay; // wait till next process call
  }
  void process(unsigned long now)
  {
    if(!_enabled) {
      return;
    }
    if(_doShow) {
      _doShow = false;
      show(now);
    }
    if(_on && now - _timeStamp > _indDelay) {
      _on = false;
      _lcd->setCursor(_col, _row);
      _lcd->print(' ');
    }
  }
private:
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

class InputDebounce
{
public:
  InputDebounce()
    : _pinIn(0)
    , _stateOn(false)
    , _timeStamp(0)
    , _debDelay(0)
    , _stateOnCount(0)
    , _enabled(false)
  {
  }
  void setup(uint8_t pinIn, unsigned long debDelay)
  {
    if(pinIn > 0) {
      _pinIn = pinIn;
      _debDelay = debDelay;
      pinMode(_pinIn, INPUT);
      _enabled = true;
    }
    else {
      _enabled = false;
    }
  }
  unsigned long process(unsigned long now)   // return pressed time if on
  {
    if(!_enabled) {
      return 0;
    }
    if(now - _timeStamp > _debDelay) {
      int value = digitalRead(_pinIn); // LOW when button pressed (on)
      if(_stateOn != !value) {
        _stateOn = !value;
        _timeStamp = now;
        if(_stateOn) {
          _stateOnCount++;
        }
        return _stateOn ? 1 : 0;
      }
    }
    return _stateOn ? now - _timeStamp : 0;
  }
  unsigned long getStateOnCount()
  {
    return _stateOnCount;
  }
private:
  uint8_t _pinIn;
  bool _stateOn;
  unsigned long _timeStamp; // last state change
  unsigned long _debDelay;
  unsigned long _stateOnCount;
  bool _enabled;
};

static ActivityLED activityLED;
static ActivityLCD activityLCD;
static IndicationLCD indicationLCD_send;
static IndicationLCD indicationLCD_recv;
static InputDebounce buttonSendDataAck;
static InputDebounce buttonSendDataNotAck;

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
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.print(F("Test TCLite"));
  lcd.setCursor(0, 1);
  lcd.print("v" TCL_VERSION " (" TCL_BUILD ")");
  
  updateLCD_TerminalState(TCL_FALSE, "");
  updateLCD_RegistrationState(TCL_TERMINAL_REGISTRATION_STATE_NOT_REGISTERED, 0, 0, 0, TCL_SYSTEM_MODE_NORMAL);
  updateLCD_Busy();
  
  //updateLCD_TerminalState(TCL_TRUE, "994110901");
  //updateLCD_RegistrationState(TCL_TERMINAL_REGISTRATION_STATE_REGISTERED, 994, 1, 2, TCL_SYSTEM_MODE_NORMAL);
  
  // TFT Adafruit ILI9340
#if TCL_LOG_TFT_ADA_ILI9340 > 0
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9340_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9340_GREEN);
  tft.setTextSize(2);
  tft.println(F("Test TCLite"));
  tft.println("v" TCL_VERSION " (" TCL_BUILD ")");
  tft.println("-------------------------");
  tft.setTextColor(ILI9340_WHITE);
  tft.setTextSize(1);
#endif
  
  // TFT Adafruit HX8357
#if TCL_LOG_TFT_ADA_HX8357 > 0
  tft.begin(HX8357D);
  tft.setRotation(1);
  tft.fillScreen(HX8357_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(HX8357_GREEN);
  tft.setTextSize(2);
  tft.println(F("Test TCLite"));
  tft.println("v" TCL_VERSION " (" TCL_BUILD ")");
  tft.println("-------------------------");
  tft.setTextColor(HX8357_WHITE);
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
  activityLED.setup(LED_BUILTIN, 500); // 500ms on, 500ms off
  
  // activity LCD
  activityLCD.setup(&lcd, LCD_ACTIVITY_COL, LCD_ACTIVITY_ROW, 1000); // 1s interval (cycle through 4 chars)
  indicationLCD_send.setup(&lcd, LCD_TCLITE_SEND_COL, LCD_TCLITE_SEND_ROW, (char)0x7e /* right arrow */, 300 * 1000); // 5min delay (not used)
  indicationLCD_recv.setup(&lcd, LCD_TCLITE_RECV_COL, LCD_TCLITE_RECV_ROW, (char)0x7f /* left arrow */, 300); // 300ms delay
  
  // buttons
  buttonSendDataAck.setup(BUTTON_PIN_SEND_DATA_ACK, BUTTON_DEBOUNCE_DELAY);
  buttonSendDataNotAck.setup(BUTTON_PIN_SEND_DATA_NOT_ACK, BUTTON_DEBOUNCE_DELAY);
  
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
  
  TCL_ReqSendDataAckConstruct(&s_reqSendDataAck, &s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_ReqSendDataAckConstruct failed");
    exit(EXIT_FAILURE);
  }
  TCL_ReqSendDataNotAckConstruct(&s_reqSendDataNotAck, &s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_ReqSendDataNotAckConstruct failed");
    exit(EXIT_FAILURE);
  }
  
  TCL_SourceAddressConstruct(&s_sourceAddress_1, &s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_SourceAddressConstruct failed");
    exit(EXIT_FAILURE);
  }
  TCL_SourceAddressConstruct(&s_sourceAddress_2, &s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_SourceAddressConstruct failed");
    exit(EXIT_FAILURE);
  }
  
  TCL_StringConstruct(&s_rfsi, s_rfsiBuffer, sizeof(s_rfsiBuffer), &s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_StringConstruct failed");
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
  
  TCL_ReqSendDataAckBusyRegisterCallback(TCL_ReqSendDataAckBusyCallback, &s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_ReqSendDataAckBusyRegisterCallback failed");
    exit(EXIT_FAILURE);
  }
  TCL_ReqSendDataNotAckBusyRegisterCallback(TCL_ReqSendDataNotAckBusyCallback, &s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_ReqSendDataNotAckBusyRegisterCallback failed");
    exit(EXIT_FAILURE);
  }
  
  TCL_RspDataAckSentRegisterCallback(TCL_RspDataAckSentCallback, &s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_RspDataAckSentRegisterCallback failed");
    exit(EXIT_FAILURE);
  }
  TCL_RspDataNotAckSentRegisterCallback(TCL_RspDataNotAckSentCallback, &s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_RspDataNotAckSentRegisterCallback failed");
    exit(EXIT_FAILURE);
  }
  
  TCL_EvtDataAckReceivedRegisterCallback(TCL_EvtDataAckReceivedCallback, &s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_EvtDataAckReceivedRegisterCallback failed");
    exit(EXIT_FAILURE);
  }
  TCL_EvtDataNotAckReceivedRegisterCallback(TCL_EvtDataNotAckReceivedCallback, &s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_EvtDataNotAckReceivedRegisterCallback failed");
    exit(EXIT_FAILURE);
  }
  
  /* start */
  
  TCL_Start(APPLICATION_ID, &s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_Start failed");
    exit(EXIT_FAILURE);
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
  
  static unsigned int buttonStateOnCount_SendDataAck = 0;
  static unsigned int buttonStateOnCount_SendDataNotAck = 0;
  
  /* TCLite */
  
  /* processing */
  
  TCL_Process(&s_error);
  if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
    TCL_LogError("TCL_Process failed");
    exit(EXIT_FAILURE);
  }
  
  if(s_send_1 && s_connected && s_registrationState == TCL_TERMINAL_REGISTRATION_STATE_REGISTERED && !s_busy) {
    s_send_1 = TCL_FALSE;
    /* TCL_ReqSendDataAck */
    Send_ReqSendDataAck(&s_error);
    if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
      exit(EXIT_FAILURE);
    }
  }
  
  if(s_send_2 && s_connected && s_registrationState == TCL_TERMINAL_REGISTRATION_STATE_REGISTERED && !s_busy) {
    s_send_2 = TCL_FALSE;
    /* TCL_ReqSendDataNotAck */
    Send_ReqSendDataNotAck(&s_error);
    if(TCL_TRUE == TCL_ErrorIsError(&s_error)) {
      exit(EXIT_FAILURE);
    }
  }
  
  // todo: check if write buffer remaining (TTCI_SL PropagateWriteToListeners)
  if(Serial1 && Serial1.available()) {
    delay(1); // [ms]
  }
  else {
    delay(s_processingInterval);
  }
  
  unsigned long now = millis();
  
  // buttons
  unsigned int buttonOnTime_SendDataAck = buttonSendDataAck.process(now);
  unsigned int buttonOnTime_SendDataNotAck = buttonSendDataNotAck.process(now);
  
  if(buttonOnTime_SendDataAck) {
    unsigned int count = buttonSendDataAck.getStateOnCount();
    if(buttonStateOnCount_SendDataAck != count) {
      buttonStateOnCount_SendDataAck = count;
      s_send_1 = TCL_TRUE;
      TCL_LogInfo("Button TCL_ReqSendDataAck pressed");
    }
    else {
      TCL_LogInfo("Button TCL_ReqSendDataAck still pressed"); // change this; log count, time
    }
  }
  if(buttonOnTime_SendDataNotAck) {
    unsigned int count = buttonSendDataNotAck.getStateOnCount();
    if(buttonStateOnCount_SendDataNotAck != count) {
      buttonStateOnCount_SendDataNotAck = count;
      s_send_2 = TCL_TRUE;
      TCL_LogInfo("Button TCL_ReqSendDataNotAck pressed");
    }
    else {
      TCL_LogInfo("Button TCL_ReqSendDataNotAck still pressed"); // change this; log count, time
    }
  }
  
  // activity LED
  activityLED.process(now);
  
  // activity LCD
  activityLCD.process(now);
  indicationLCD_send.process(now);
  indicationLCD_recv.process(now);
}

