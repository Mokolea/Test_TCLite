/*
  Test TCLite on Arduino Due
  
  Mario Ban, 12.2014
  
*/

#include <Wire.h>
#include "LiquidCrystal_I2C.h"

#include "TCLite.h"

static LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 20 chars and 4 line display

#define APPLICATION_ID         100
#define LOG_HEX_DUMP_COLUMNS   32

static TCL_Error s_error;
static TCL_UInt32 s_processingInterval; /* [ms] */

static TCL_Bool s_connected = TCL_FALSE;
static TCL_TerminalRegistrationStateType s_registrationState = TCL_TERMINAL_REGISTRATION_STATE_NOT_REGISTERED;

class ActivityLED
{
public:
  ActivityLED()
   : _pin(-1)
   , _interval(1000)
   , _last(0)
   , _toggle(false)
  {}
  void setup(int pin, unsigned long interval) {
    _pin = pin;
    _interval = interval;
    pinMode(_pin, OUTPUT);
  }
  void process(unsigned long now) {
    if(now > _last + _interval) {
      _last = now;
      if(_toggle) {
        digitalWrite(_pin, HIGH);
      }
      else {
        digitalWrite(_pin, LOW);
      }
      _toggle = !_toggle;
    }
  }
private:
  int _pin;
  unsigned long _interval;
  unsigned long _last;
  bool _toggle;
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
  {}
  void setup(LiquidCrystal_I2C *lcd, unsigned char col, unsigned char row, unsigned long interval) {
    _lcd = lcd;
    _col = col;
    _row = row;
    _interval = interval;
  }
  void process(unsigned long now) {
    if(now > _last + _interval) {
      _last = now;
      _lcd->setCursor(_col, _row);
      if((_count+3) % 4 == 0) {
        _lcd->print("-");
      }
      else if((_count+2) % 4 == 0) {
        _lcd->print("\\");
      }
      else if((_count+1) % 4 == 0) {
        _lcd->print("|");
      }
      else if(_count % 4 == 0) {
        _lcd->print("/");
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
};

static ActivityLED activityLED;

static ActivityLCD activityLCD;

static void TCL_EvtTerminalStateCallback(const TCL_EvtTerminalState* event, TCL_Error* error)
{
  TCL_EventNumber eventNumber;
  TCL_Bool connected;
  TCL_Address address;
  const TCL_Data* serialNumber;
  const TCL_Data* softwareVersion;
  TCL_String rfsi;
  TCL_Char rfsiBuffer[TCL_ADDRESS_RFSI_STRING_BUFFER_SIZE];
  TCL_Char buffer[200];
  
  /* get information */
  
  eventNumber = TCL_EvtTerminalStateGetEventNumber(event);
  connected = TCL_EvtTerminalStateGetConnected(event);
  address = TCL_EvtTerminalStateGetAddress(event);
  serialNumber = TCL_EvtTerminalStateGetSerialNumber(event);
  softwareVersion = TCL_EvtTerminalStateGetSoftwareVersion(event);
  
  s_connected = connected;
  
  TCL_StringConstruct(&rfsi, rfsiBuffer, sizeof(rfsiBuffer), error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_StringConstruct failed");
    return;
  }
  TCL_AddressGetRFSI(&address, &rfsi, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_AddressGetRFSI failed");
    return;
  }
  
  TCL_SNPrintf(buffer, sizeof(buffer), "TCL_EvtTerminalStateCallback"
               ": eventNumber: %u"
               ", connected: %d"
               ", RFSI: '%s'"
               , eventNumber
               , connected
               , TCL_StringGetString(&rfsi)
              );
  TCL_LogInfo(buffer);
  
  TCL_LogInfo("data \\ serialNumber (hex-dump)");
  TCL_DataPrintHexDump_Info(serialNumber, LOG_HEX_DUMP_COLUMNS);
  TCL_LogInfo("data /");
  
  TCL_LogInfo("data \\ softwareVersion (hex-dump)");
  TCL_DataPrintHexDump_Info(softwareVersion, LOG_HEX_DUMP_COLUMNS);
  TCL_LogInfo("data /");
}

static void TCL_EvtRegistrationStateCallback(const TCL_EvtRegistrationState* event, TCL_Error* error)
{
  TCL_EventNumber eventNumber;
  TCL_TerminalRegistrationStateType registrationState;
  TCL_UInt16 rBaseNetwork;
  TCL_UInt8 rswIdentifier;
  TCL_UInt8 bsIdentifier;
  TCL_SystemOperatingModeType systemMode;
  TCL_RadioFieldLevelType radioFieldLevel;
  TCL_Char buffer[220];
  
  /* get information */
  
  eventNumber = TCL_EvtRegistrationStateGetEventNumber(event);
  registrationState = TCL_EvtRegistrationStateGetRegistrationState(event);
  rBaseNetwork = TCL_EvtRegistrationStateGetRBaseNetwork(event);
  rswIdentifier = TCL_EvtRegistrationStateGetRswIdentifier(event);
  bsIdentifier = TCL_EvtRegistrationStateGetBsIdentifier(event);
  systemMode = TCL_EvtRegistrationStateGetSystemMode(event);
  radioFieldLevel = TCL_EvtRegistrationStateGetRadioFieldLevel(event);
  
  s_registrationState = registrationState;
  
  TCL_SNPrintf(buffer, sizeof(buffer), "TCL_EvtRegistrationStateCallback"
               ": eventNumber: %u"
               ", registrationState: %d"
               ", rBaseNetwork: %d"
               ", rswIdentifier: %d"
               ", bsIdentifier: %d"
               ", systemMode: %d"
               ", radioFieldLevel: %d"
               , eventNumber
               , registrationState
               , rBaseNetwork
               , rswIdentifier
               , bsIdentifier
               , systemMode
               , radioFieldLevel
              );
  TCL_LogInfo(buffer);
  
  TCL_ErrorSetErrorCode(error, TCL_ERROR_NONE);
}

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
  
  // activity LED
  activityLED.setup(LED_BUILTIN, 500); // pin out 13; 500ms on, 500ms off
  
  // activity LCD
  activityLCD.setup(&lcd, 19, 3, 500); // col 20, row 4; 500ms interval
  
  // LCD
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.print("Test TCLite");
  lcd.setCursor(0, 1);
  lcd.print("v" TCL_VERSION " (" TCL_BUILD ")");
  
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

