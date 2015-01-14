/*
  Test TCLite on Arduino Due
  
  Mario Ban, 01.2015
  
*/

static void TCL_RspDataAckSentCallback(const TCL_RspDataAckSent* response, TCL_Error* error)
{
  TCL_RequestId requestId;
  const TCL_Result* result;
  TCL_ResultErrorCode resultErrorCode;
  const TCL_String* resultErrorInfo;
  TCL_Char buffer[240];
  
  requestId = TCL_RspDataAckSentGetRequestId(response);
  result = TCL_RspDataAckSentGetResult(response);
  
  resultErrorCode = TCL_ResultGetErrorCode(result);
  resultErrorInfo = TCL_ResultGetErrorInfo(result);
  
  TCL_SNPrintf(buffer, sizeof(buffer), "TCL_RspDataAckSentCallback"
               ": requestId: %u"
               ", resultErrorCode: %d"
               ", resultErrorInfo: '%s'"
               , requestId
               , resultErrorCode
               , TCL_StringGetString(resultErrorInfo)
              );
  TCL_LogInfo(buffer);
  
  // LCD
  //updateLCD_DataIndication(TCL_FALSE, TCL_TRUE);
  indicationLCD_send.hide();
  
  TCL_ErrorSetErrorCode(error, TCL_ERROR_NONE);
}

static void TCL_EvtDataAckReceivedCallback(const TCL_EvtDataAckReceived* event, TCL_Error* error)
{
  TCL_EventNumber eventNumber;
  TCL_SourceAddress sourceAddress;
  TCL_SourceAddressType sourceAddressType;
  TCL_Address address;
  TCL_UInt32 addressBinary;
  TCL_String rfsi;
  TCL_Char rfsiBuffer[TCL_ADDRESS_RFSI_STRING_BUFFER_SIZE];
  TCL_MandatoryEncryption mandatoryEncryption;
  TCL_Priority priority;
  TCL_Compression compression;
  const TCL_Data* data;
  TCL_UInt32 dataSize = 0;
  TCL_Char buffer[200];
  
  TCL_Data  dataCompression;
  //TCL_UInt8 dataCompressionBuffer[800 * 1000];
  TCL_UInt8 dataCompressionBuffer[8 * 10];
  
  // LCD
  //updateLCD_DataIndication(TCL_TRUE, TCL_FALSE);
  indicationLCD_recv.show(millis());
  
  TCL_DataConstruct(&dataCompression, dataCompressionBuffer, sizeof(dataCompressionBuffer), error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_DataConstruct failed");
    return;
  }
  
  /* get information */
  
  eventNumber = TCL_EvtDataAckReceivedGetEventNumber(event);
  sourceAddress = TCL_EvtDataAckReceivedGetFromAddress(event);
  sourceAddressType = TCL_SourceAddressGetType(&sourceAddress);
  address = TCL_SourceAddressGetAddress(&sourceAddress);
  addressBinary = TCL_AddressGetNumber(&address);
  TCL_StringConstruct(&rfsi, rfsiBuffer, sizeof(rfsiBuffer), error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_StringConstruct failed");
    return;
  }
  if(sourceAddressType == TCL_DATASAT_RFSI) {
    TCL_AddressGetRFSI(&address, &rfsi, error);
    if(TCL_TRUE == TCL_ErrorIsError(error)) {
      TCL_LogError("TCL_AddressGetRFSI failed");
      return;
    }
  }
  mandatoryEncryption = TCL_EvtDataAckReceivedGetMandatoryEncryption(event);
  priority = TCL_EvtDataAckReceivedGetPriority(event);
  compression = TCL_EvtDataAckReceivedGetCompressed(event);
  
  /* handle compression */
  if(compression == TCL_COMPRESSION_ON) {
#if TCL_SERVICE_DATA_COMPRESSION > 0
    TCL_DataCompression_Decompress(&dataCompression, TCL_EvtDataAckReceivedGetData(event), error);
    if(TCL_TRUE == TCL_ErrorIsError(error)) {
      TCL_LogError("TCL_DataCompression_Decompress failed");
      return;
    }
#endif /* TCL_SERVICE_DATA_COMPRESSION */
    data = &dataCompression;
  }
  else {
    data = TCL_EvtDataAckReceivedGetData(event);
  }
  
  if(TCL_DATA_ACK_SEND_BACK) {
    s_send_1 = TCL_TRUE;
    TCL_SourceAddressClone(&s_sourceAddress_1, &sourceAddress, error);
    if(TCL_TRUE == TCL_ErrorIsError(error)) {
      TCL_LogError("TCL_SourceAddressClone failed");
      return;
    }
  }
  
  dataSize = TCL_DataGetSize(data);
  
  TCL_SNPrintf(buffer, sizeof(buffer), "TCL_EvtDataAckReceivedCallback"
               ": eventNumber: %u"
               ", sourceAddressType: %d"
               ", addressBinary: 0x%08x"
               ", RFSI: '%s'"
               ", mandatoryEncryption: %d"
               ", priority: %d"
               ", compression: %d"
               ", dataSize: %u"
               , eventNumber
               , sourceAddressType
               , addressBinary
               , TCL_StringGetString(&rfsi)
               , mandatoryEncryption
               , priority
               , compression
               , dataSize
              );
  TCL_LogInfo(buffer);
  TCL_LogInfo("data \\ (hex-dump)");
  TCL_DataPrintHexDump_Info(data, LOG_HEX_DUMP_COLUMNS);
  TCL_LogInfo("data /");
  
  // LCD
  updateLCD_Data(data);
  //updateLCD_DataIndication(TCL_FALSE, TCL_FALSE);
}

static void Send_ReqSendDataAck(TCL_Error* error)
{
  TCL_Data  data;
  //TCL_UInt8 dataBuffer[1500]; /* TCL_REQ_SEND_DATA_ACK_DATA_SIZE_MAX */
  TCL_UInt8 dataBuffer[150];
  TCL_UInt8 data_test1[] = {'H', 'e', 'l', 'l', 'o'};
  TCL_UInt32 i;
  TCL_UInt8 fill = 'o'; /* test DLEs (these will be doubled): 0x10 */
  TCL_UInt32 dataSize = 0;
  TCL_DataAddressType dataAddressType;
  TCL_Address address;
  TCL_Compression compression;
  TCL_Result result;
  TCL_ResultErrorCode resultErrorCode;
  const TCL_String* resultErrorInfo;
  TCL_Char buffer[240];
  
  TCL_Data  dataCompression;
  //TCL_UInt8 dataCompressionBuffer[800 * 1000];
  TCL_UInt8 dataCompressionBuffer[8 * 10];
  
  // LCD
  //updateLCD_DataIndication(TCL_TRUE, TCL_TRUE);
  indicationLCD_send.show();
  
  compression = TCL_COMPRESSION_OFF;
  
  TCL_DataConstruct(&data, dataBuffer, sizeof(dataBuffer), error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_DataConstruct failed");
    return;
  }
  TCL_DataSetData(&data, data_test1, sizeof(data_test1), error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_DataSetData failed");
    return;
  }
  
  TCL_DataConstruct(&dataCompression, dataCompressionBuffer, sizeof(dataCompressionBuffer), error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_DataConstruct failed");
    return;
  }
  
#if TCL_DATA_ACK_SEND_CASE < 1
#error "test-case not defined"
#endif
  
#if TCL_DATA_ACK_SEND_CASE == 1
  /* test 1: max 1472-4 bytes data (DTAP segmented) */
  for(i = 0; i < 1472 - 4 - 5 - TCL_REDUCE_BUFFER_SIZE_DTAP_MAX_BY; i++) {
    TCL_DataAppend1(&data, &fill, 1, error);
    if(TCL_TRUE == TCL_ErrorIsError(error)) {
      return;
    }
  }
#endif
  
#if TCL_DATA_ACK_SEND_CASE == 2
  /* test 2: 100 bytes data (DTAP not segmented) */
  for(i = 0; i < 100 - 5; i++) {
    TCL_DataAppend1(&data, &fill, 1, error);
    if(TCL_TRUE == TCL_ErrorIsError(error)) {
      TCL_LogError("TCL_DataAppend1 failed");
      return;
    }
  }
#endif
  
#if TCL_DATA_ACK_SEND_CASE == 3
  /* test 3: (1..)8 bytes (TCL_TETRAPOL_SERVICE_UDT2DAS_UL_SHORT) */
  (void)i;
  (void)fill;
  TCL_DataConsumedSize(&data, 1, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    return;
  }
#endif
  
#if TCL_DATA_ACK_SEND_CASE == 4
  /* test 4: 9 bytes (TCL_TETRAPOL_SERVICE_UDT2DAS_UL_POLLING) */
  (void)i;
  (void)fill;
#endif
  
#if TCL_DATA_ACK_SEND_CASE == 5
  /* test 5: compression */
  (void)fill;
  for(i = 0; i < sizeof(dataCompressionBuffer) / sizeof(data_test1); i++) {
    TCL_DataAppend1(&dataCompression, data_test1, sizeof(data_test1), error);
    if(TCL_TRUE == TCL_ErrorIsError(error)) {
      return;
    }
  }
  TCL_DataCompression_Compress(&data, &dataCompression, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_DataCompression_Compress failed");
    return;
  }
  compression = TCL_COMPRESSION_ON;
#endif
  
#if TCL_DATA_ACK_SEND_CASE > 5
#error "test-case not defined"
#endif
  
  TCL_ResultConstruct(&result, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ResultConstruct failed");
    return;
  }
  
  /* setup TCL_ReqSendDataAck */
  
  TCL_ReqSendDataAckSetRequestId(&s_reqSendDataAck, s_requestId++, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataAckSetRequestId failed");
    return;
  }
  TCL_ReqSendDataAckSetMandatoryEncryption(&s_reqSendDataAck, TCL_MANDATORY_ENCRYPTION_OFF, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataAckSetMandatoryEncryption failed");
    return;
  }
  TCL_ReqSendDataAckSetPriority(&s_reqSendDataAck, TCL_DATAP_NORMAL, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataAckSetPriority failed");
    return;
  }
  TCL_ReqSendDataAckSetCompression(&s_reqSendDataAck, compression, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataAckSetCompression failed");
    return;
  }
  TCL_ReqSendDataAckSetTransmissionType(&s_reqSendDataAck, TCL_TRANSMISSION_TYPE_RELIABLE, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataAckSetTransmissionType failed");
    return;
  }
  
  dataAddressType = TCL_DATAAT_APP;
  if(TCL_SourceAddressGetType(&s_sourceAddress_1) == TCL_DATASAT_RFSI) {
    /* send back to source address */
    dataAddressType = TCL_DATAAT_RFSI;
  }
  TCL_ReqSendDataAckSetDataAddressType(&s_reqSendDataAck, dataAddressType, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataAckSetDataAddressType failed");
    return;
  }
  address = TCL_SourceAddressGetAddress(&s_sourceAddress_1);
  /* test: send to self */
  /*TCL_StringSetString(&s_rfsi, "994110901", error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_StringSetString failed");
    return;
  }
  TCL_AddressSetRFSI(&address, &s_rfsi, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_AddressSetRFSI failed");
    return;
  }*/
  TCL_ReqSendDataAckSetAddress(&s_reqSendDataAck, &address, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataAckSetAddress failed");
    return;
  }
  
  TCL_ReqSendDataAckSetData(&s_reqSendDataAck, &data, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataAckSetData failed");
    return;
  }
  
  dataSize = TCL_DataGetSize(&data);
  
  /* send request */
  
  TCL_ReqSendDataAckSendRequest(&s_reqSendDataAck, &result, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataAckSendRequest failed (error)");
    return;
  }
  if(TCL_TRUE == TCL_ResultIsError(&result)) {
    TCL_LogError("TCL_ReqSendDataAckSendRequest failed (result)");
  }
  
  resultErrorCode = TCL_ResultGetErrorCode(&result);
  resultErrorInfo = TCL_ResultGetErrorInfo(&result);
  
  TCL_SNPrintf(buffer, sizeof(buffer), "TCL_ReqSendDataAckSendRequest"
               ": requestId: %u"
               ", dataSize: %u"
               ", resultErrorCode: %d"
               ", resultErrorInfo: '%s'"
               , TCL_ReqSendDataAckGetRequestId(&s_reqSendDataAck)
               , dataSize
               , resultErrorCode
               , TCL_StringGetString(resultErrorInfo)
              );
  TCL_LogInfo(buffer);
}

