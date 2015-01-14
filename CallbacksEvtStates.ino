/*
  Test TCLite on Arduino Due
  
  Mario Ban, 01.2015
  
*/

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
  
  // LCD
  updateLCD_TerminalState(connected, TCL_StringGetString(&rfsi));
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
  
  // LCD
  updateLCD_RegistrationState(registrationState, rBaseNetwork, rswIdentifier, bsIdentifier, systemMode);
  
  TCL_ErrorSetErrorCode(error, TCL_ERROR_NONE);
}

