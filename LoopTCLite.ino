/*
  Test TCLite on Arduino Due

  Mario Ban, 05.2015

  GNU General Public License v2.0
  Copyright (C) 2015 Mario Ban
*/

static void loop_TCLite()
{
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
}

