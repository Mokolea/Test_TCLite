/*
  Test TCLite on Arduino Due

  Mario Ban, 05.2015

  GNU General Public License v2.0
  Copyright (C) 2015 Mario Ban
*/

void setup_TCLite()
{
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
