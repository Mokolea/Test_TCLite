/*
  Test TCLite on Arduino Due

  Mario Ban, 01.2015

  GNU General Public License v2.0
  Copyright (C) 2015 Mario Ban
*/

void updateLCD_TerminalState(TCL_Bool connected, const TCL_Char* RFSI)
{
  // LCD
  s_lcd.setCursor(LCD_TERMINAL_STATE_COL, LCD_TERMINAL_STATE_ROW);
  s_lcd.print("                    "); /* 20 spaces */
  s_lcd.setCursor(LCD_TERMINAL_STATE_COL, LCD_TERMINAL_STATE_ROW);
  
  if(!connected) {
    s_lcd.print("not connected");
  }
  else {
    s_lcd.print(RFSI);
  }
}

void updateLCD_RegistrationState(TCL_TerminalRegistrationStateType registrationState,
                                        TCL_UInt16 rBaseNetwork, TCL_UInt8 rswIdentifier, TCL_UInt8 bsIdentifier, TCL_SystemOperatingModeType systemMode)
{
  // LCD
  s_lcd.setCursor(LCD_REGISTRATION_STATE_COL, LCD_REGISTRATION_STATE_ROW);
  s_lcd.print("                    "); /* 20 spaces */
  s_lcd.setCursor(LCD_REGISTRATION_STATE_COL, LCD_REGISTRATION_STATE_ROW);
  
  if(registrationState != TCL_TERMINAL_REGISTRATION_STATE_REGISTERED) {
    s_lcd.print("not registered");
  }
  else {
    s_lcd.print(rBaseNetwork, DEC);
    s_lcd.print(" ");
    s_lcd.print(rswIdentifier, DEC);
    s_lcd.print(" ");
    s_lcd.print(bsIdentifier, DEC);
    s_lcd.print(" ");
    if(systemMode == TCL_SYSTEM_MODE_NORMAL) {
      s_lcd.print("normal");
    }
    else if(systemMode == TCL_SYSTEM_MODE_BN_DISCONNECTED_FBM1) {
      s_lcd.print("FBM 1");
    }
    else if(systemMode == TCL_SYSTEM_MODE_MSW_DISCONNECTED_FBM2) {
      s_lcd.print("FBM 2");
    }
    else if(systemMode == TCL_SYSTEM_MODE_RSW_DISCONNECTED_FBM31) {
      s_lcd.print("FBM 3.1");
    }
    else if(systemMode == TCL_SYSTEM_MODE_BSC_DISCONNECTED_FBM32) {
      s_lcd.print("FBM 3.2");
    }
    else {
      s_lcd.print("unknown");
    }
  }
}

void updateLCD_Busy()
{
  TCL_Bool busy = s_busy_1 || s_busy_2;
  
  // LCD
  s_lcd.setCursor(LCD_TCLITE_BUSY_COL, LCD_TCLITE_BUSY_ROW);
  
  if(busy && !s_busy) {
    s_lcd.print("B");
  }
  else if(!busy && s_busy) {
    s_lcd.print(" ");
  }
  s_busy = busy;
}

#if 0
void updateLCD_DataIndication(TCL_Bool indStart, TCL_Bool dirSend)
{
  char c = ' ';
  
  // LCD
  if(dirSend) {
    s_lcd.setCursor(LCD_TCLITE_SEND_COL, LCD_TCLITE_SEND_ROW);
    if(indStart) {
      c = (char)0x7e; /* right arrow */
    }
  }
  else {
    s_lcd.setCursor(LCD_TCLITE_RECV_COL, LCD_TCLITE_RECV_ROW);
    if(indStart) {
      c = (char)0x7f; /* left arrow */
    }
  }
  s_lcd.print(c);
}
#endif

void updateLCD_Data(const TCL_Data* data)
{
  // LCD
  s_lcd.setCursor(LCD_TCLITE_DATA_RECV_COL, LCD_TCLITE_DATA_RECV_ROW);
  s_lcd.print("                    "); /* 20 spaces */
  s_lcd.setCursor(LCD_TCLITE_DATA_RECV_COL, LCD_TCLITE_DATA_RECV_ROW);
  
  TCL_UInt8* dataPointer = TCL_DataGetDataPointer(data);
  
#if 0
  for(TCL_UInt32 i = 0; i < TCL_DataGetSize(data) && i < 4; i++) {
    char hex[6];
    sprintf(hex, "0x%02x ", dataPointer[i]);
    s_lcd.print(hex);
  }
#endif
  
  // print max first 20 data bytes
  for(TCL_UInt32 i = 0; i < TCL_DataGetSize(data) && i < 20; i++) {
    if(dataPointer[i] >= 0x20 && dataPointer[i] <= 0x7e) {
      s_lcd.print((TCL_Char)dataPointer[i]); /* printable */
    }
    else {
      s_lcd.print("."); /* substitute non-printable */
    }
  }
}

