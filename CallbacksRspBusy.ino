/*
  Test TCLite on Arduino Due
  
  Mario Ban, 01.2015
  
*/

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

static void TCL_ReqSendDataAckBusyCallback(TCL_Bool busy, TCL_Error* error)
{
  TCL_Char buffer[100];
  
  TCL_SNPrintf(buffer, sizeof(buffer), "TCL_ReqSendDataAckBusyCallback"
               ": busy: %d"
               , busy
              );
  TCL_LogInfo(buffer);
  
  if(busy) {
    s_busyCount++;
  }
  else {
    s_busyCount--;
  }
  
  // LCD
  updateLCD_Busy();
  
  TCL_ErrorSetErrorCode(error, TCL_ERROR_NONE);
}

static void TCL_ReqSendDataNotAckBusyCallback(TCL_Bool busy, TCL_Error* error)
{
  TCL_Char buffer[100];
  
  TCL_SNPrintf(buffer, sizeof(buffer), "TCL_ReqSendDataNotAckBusyCallback"
               ": busy: %d"
               , busy
              );
  TCL_LogInfo(buffer);
  
  if(busy) {
    s_busyCount++;
  }
  else {
    s_busyCount--;
  }
  
  // LCD
  updateLCD_Busy();
  
  TCL_ErrorSetErrorCode(error, TCL_ERROR_NONE);
}

