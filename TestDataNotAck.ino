/*
  Test TCLite on Arduino Due
  
  Mario Ban, 01.2015
  
*/

static void TCL_RspDataNotAckSentCallback(const TCL_RspDataNotAckSent* response, TCL_Error* error)
{
  TCL_RequestId requestId;
  const TCL_Result* result;
  TCL_ResultErrorCode resultErrorCode;
  const TCL_String* resultErrorInfo;
  TCL_Char buffer[240];
  
  requestId = TCL_RspDataNotAckSentGetRequestId(response);
  result = TCL_RspDataNotAckSentGetResult(response);
  
  resultErrorCode = TCL_ResultGetErrorCode(result);
  resultErrorInfo = TCL_ResultGetErrorInfo(result);
  
  TCL_SNPrintf(buffer, sizeof(buffer), "TCL_RspDataNotAckSentCallback"
               ": requestId: %u"
               ", resultErrorCode: %d"
               ", resultErrorInfo: '%s'"
               , requestId
               , resultErrorCode
               , TCL_StringGetString(resultErrorInfo)
              );
  TCL_LogInfo(buffer);
  
  TCL_ErrorSetErrorCode(error, TCL_ERROR_NONE);
}

static void TCL_EvtDataNotAckReceivedCallback(const TCL_EvtDataNotAckReceived* event, TCL_Error* error)
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
  
  TCL_DataConstruct(&dataCompression, dataCompressionBuffer, sizeof(dataCompressionBuffer), error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_DataConstruct failed");
    return;
  }
  
  /* get information */
  
  eventNumber = TCL_EvtDataNotAckReceivedGetEventNumber(event);
  sourceAddress = TCL_EvtDataNotAckReceivedGetFromAddress(event);
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
  mandatoryEncryption = TCL_EvtDataNotAckReceivedGetMandatoryEncryption(event);
  priority = TCL_EvtDataNotAckReceivedGetPriority(event);
  compression = TCL_EvtDataNotAckReceivedGetCompressed(event);
  
  /* handle compression */
  if(compression == TCL_COMPRESSION_ON) {
#if TCL_SERVICE_DATA_COMPRESSION > 0
    TCL_DataCompression_Decompress(&dataCompression, TCL_EvtDataNotAckReceivedGetData(event), error);
    if(TCL_TRUE == TCL_ErrorIsError(error)) {
      TCL_LogError("TCL_DataCompression_Decompress failed");
      return;
    }
#endif /* TCL_SERVICE_DATA_COMPRESSION */
    data = &dataCompression;
  }
  else {
    data = TCL_EvtDataNotAckReceivedGetData(event);
  }
  
  if(TCL_DATA_NOT_ACK_SEND_BACK) {
    s_send_2 = TCL_TRUE;
    TCL_SourceAddressClone(&s_sourceAddress_2, &sourceAddress, error);
    if(TCL_TRUE == TCL_ErrorIsError(error)) {
      TCL_LogError("TCL_SourceAddressClone failed");
      return;
    }
  }
  
  dataSize = TCL_DataGetSize(data);
  
  TCL_SNPrintf(buffer, sizeof(buffer), "TCL_EvtDataNotAckReceivedCallback"
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
}

static void Send_ReqSendDataNotAck(TCL_Error* error)
{
  TCL_Data  data;
  TCL_UInt8 dataBuffer[1500]; /* TCL_REQ_SEND_DATA_NOT_ACK_DATA_SIZE_MAX */
  TCL_UInt8 data_test1[] = {'H', 'e', 'l', 'l', 'o'};
  TCL_UInt32 i;
  TCL_UInt8 fill = 'o'; /* test DLEs (these will be doubled): 0x10 */
  TCL_UInt32 dataSize = 0;
  TCL_DataAddressType dataAddressType;
  TCL_Address address;
  TCL_OperationalGroupId ogId;
  TCL_Compression compression;
  TCL_Result result;
  TCL_ResultErrorCode resultErrorCode;
  const TCL_String* resultErrorInfo;
  TCL_Char buffer[240];
  
  TCL_Data  dataCompression;
  //TCL_UInt8 dataCompressionBuffer[800 * 1000];
  TCL_UInt8 dataCompressionBuffer[8 * 10];
  
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
  
  TCL_OperationalGroupIdConstruct(&ogId, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_OperationalGroupIdConstruct failed");
    return;
  }
  TCL_OperationalGroupIdSetOperationalGroupNumber(&ogId, 104, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_OperationalGroupIdSetOperationalGroupNumber failed");
    return;
  }
  TCL_OperationalGroupIdSetRnNumber(&ogId, 994, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_OperationalGroupIdSetRnNumber failed");
    return;
  }
  
#if TCL_DATA_NOT_ACK_SEND_CASE < 1
#error "test-case not defined"
#endif
  
#if TCL_DATA_NOT_ACK_SEND_CASE == 1
  /* test 1: max 1472-4 bytes data (DTAP segmented) */
  for(i = 0; i < 1472 - 4 - 5 - TCL_REDUCE_BUFFER_SIZE_DTAP_MAX_BY; i++) {
    TCL_DataAppend1(&data, &fill, 1, error);
    if(TCL_TRUE == TCL_ErrorIsError(error)) {
      return;
    }
  }
#endif
  
#if TCL_DATA_NOT_ACK_SEND_CASE == 2
  /* test 2: 100 bytes data (DTAP not segmented) */
  for(i = 0; i < 100 - 5; i++) {
    TCL_DataAppend1(&data, &fill, 1, error);
    if(TCL_TRUE == TCL_ErrorIsError(error)) {
      TCL_LogError("TCL_DataAppend1 failed");
      return;
    }
  }
#endif
  
#if TCL_DATA_NOT_ACK_SEND_CASE == 3
  /* test 3: (1..)8 bytes (TCL_TETRAPOL_SERVICE_UDT2DAS_UL_SHORT) */
  (void)i;
  (void)fill;
  TCL_DataConsumedSize(&data, 1, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    return;
  }
#endif
  
#if TCL_DATA_NOT_ACK_SEND_CASE == 4
  /* test 4: 9 bytes (TCL_TETRAPOL_SERVICE_UDT2DAS_UL_POLLING) */
  (void)i;
  (void)fill;
#endif
  
#if TCL_DATA_NOT_ACK_SEND_CASE == 5
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
  
#if TCL_DATA_NOT_ACK_SEND_CASE > 5
#error "test-case not defined"
#endif
  
  TCL_ResultConstruct(&result, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ResultConstruct failed");
    return;
  }
  
  /* setup TCL_ReqSendDataNotAck */
  
  TCL_ReqSendDataNotAckSetRequestId(&s_reqSendDataNotAck, s_requestId++, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataNotAckSetRequestId failed");
    return;
  }
  TCL_ReqSendDataNotAckSetMandatoryEncryption(&s_reqSendDataNotAck, TCL_MANDATORY_ENCRYPTION_OFF, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataNotAckSetMandatoryEncryption failed");
    return;
  }
  TCL_ReqSendDataNotAckSetPriority(&s_reqSendDataNotAck, TCL_DATAP_NORMAL, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataNotAckSetPriority failed");
    return;
  }
  TCL_ReqSendDataNotAckSetCompression(&s_reqSendDataNotAck, compression, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataNotAckSetCompression failed");
    return;
  }
  TCL_ReqSendDataNotAckSetTransmissionType(&s_reqSendDataNotAck, TCL_TRANSMISSION_TYPE_EFFICIENT, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataNotAckSetTransmissionType failed");
    return;
  }
  
  dataAddressType = TCL_DATAAT_APP; /* test: TCL_DATAAT_OG */
  if(TCL_SourceAddressGetType(&s_sourceAddress_2) == TCL_DATASAT_RFSI) {
    /* send back to source address */
    dataAddressType = TCL_DATAAT_RFSI;
  }
  TCL_ReqSendDataNotAckSetDataAddressType(&s_reqSendDataNotAck, dataAddressType, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataNotAckSetDataAddressType failed");
    return;
  }
  address = TCL_SourceAddressGetAddress(&s_sourceAddress_2);
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
  TCL_ReqSendDataNotAckSetAddress(&s_reqSendDataNotAck, &address, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataNotAckSetAddress failed");
    return;
  }
  TCL_ReqSendDataNotAckSetOGId(&s_reqSendDataNotAck, &ogId, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataNotAckSetOGId failed");
    return;
  }
  
  TCL_ReqSendDataNotAckSetData(&s_reqSendDataNotAck, &data, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataNotAckSetData failed");
    return;
  }
  
  dataSize = TCL_DataGetSize(&data);
  
  /* send request */
  
  TCL_ReqSendDataNotAckSendRequest(&s_reqSendDataNotAck, &result, error);
  if(TCL_TRUE == TCL_ErrorIsError(error)) {
    TCL_LogError("TCL_ReqSendDataNotAckSendRequest failed (error)");
    return;
  }
  if(TCL_TRUE == TCL_ResultIsError(&result)) {
    TCL_LogError("TCL_ReqSendDataNotAckSendRequest failed (result)");
  }
  
  resultErrorCode = TCL_ResultGetErrorCode(&result);
  resultErrorInfo = TCL_ResultGetErrorInfo(&result);
  
  TCL_SNPrintf(buffer, sizeof(buffer), "TCL_ReqSendDataNotAckSendRequest"
               ": requestId: %u"
               ", dataSize: %u"
               ", resultErrorCode: %d"
               ", resultErrorInfo: '%s'"
               , TCL_ReqSendDataNotAckGetRequestId(&s_reqSendDataNotAck)
               , dataSize
               , resultErrorCode
               , TCL_StringGetString(resultErrorInfo)
              );
  TCL_LogInfo(buffer);
}

