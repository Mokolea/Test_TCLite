/*
  Test TCLite on Arduino Due
  
  Mario Ban, 01.2015
  
*/

void updateLCD_TerminalState(TCL_Bool connected, TCL_Char* RFSI)
{
  // LCD
  lcd.setCursor(LCD_TERMINAL_STATE_COL, LCD_TERMINAL_STATE_ROW);
  lcd.print("                    "); /* 20 spaces */
  lcd.setCursor(LCD_TERMINAL_STATE_COL, LCD_TERMINAL_STATE_ROW);
  
  if(!connected) {
    lcd.print("not connected");
  }
  else {
    lcd.print(RFSI);
  }
}

void updateLCD_RegistrationState(TCL_TerminalRegistrationStateType registrationState,
  TCL_UInt16 rBaseNetwork, TCL_UInt8 rswIdentifier, TCL_UInt8 bsIdentifier)
{
  // LCD
  lcd.setCursor(LCD_REGISTRATION_STATE_COL, LCD_REGISTRATION_STATE_ROW);
  lcd.print("                    "); /* 20 spaces */
  lcd.setCursor(LCD_REGISTRATION_STATE_COL, LCD_REGISTRATION_STATE_ROW);
  
  if(registrationState != TCL_TERMINAL_REGISTRATION_STATE_REGISTERED) {
    lcd.print("not registered");
  }
  else {
    lcd.print(rBaseNetwork, DEC);
    lcd.print(" ");
    lcd.print(rswIdentifier, DEC);
    lcd.print(" ");
    lcd.print(bsIdentifier, DEC);
  }
}

void updateLCD_Busy()
{
  TCL_Bool busy = (s_busyCount != 0);
  
  // LCD
  lcd.setCursor(LCD_TCLITE_BUSY_COL, LCD_TCLITE_BUSY_ROW);
  
  if(busy && !s_busy) {
    lcd.print("B");
  }
  else if(!busy && s_busy) {
    lcd.print(" ");
  }
  s_busy = busy;
}

void updateLCD_Data(const TCL_Data* data)
{
}

