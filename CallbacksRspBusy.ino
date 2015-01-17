/*
  Test TCLite on Arduino Due

  Mario Ban, 01.2015

*/

static void TCL_ReqSendDataAckBusyCallback(TCL_Bool busy, TCL_Error* error)
{
  TCL_Char buffer[100];
  
  TCL_SNPrintf(buffer, sizeof(buffer), "TCL_ReqSendDataAckBusyCallback"
               ": busy: %d"
               , busy
              );
  TCL_LogInfo(buffer);
  
  s_busy_1 = busy;
  
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
  
  s_busy_2 = busy;
  
  // LCD
  updateLCD_Busy();
  
  TCL_ErrorSetErrorCode(error, TCL_ERROR_NONE);
}

