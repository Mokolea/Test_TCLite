/*
  Test TCLite on Arduino Due

  Mario Ban, 10.2015

  GNU General Public License v2.0
  Copyright (C) 2015 Mario Ban
*/

void handleButtons(unsigned long now)
{
  static unsigned int buttonStateOnCount_SendDataAck = 0;
  static unsigned int buttonStateOnCount_SendDataNotAck = 0;
  
  // buttons: poll button state, return on-time [ms] if pressed (debounced)
  unsigned int buttonOnTime_SendDataAck = s_buttonSendDataAck.process(now);
  unsigned int buttonOnTime_SendDataNotAck = s_buttonSendDataNotAck.process(now);
  
  // TODO change this to using callbacks
  
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
}

