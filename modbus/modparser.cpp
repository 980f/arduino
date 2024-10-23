#include "modparser.h"

ModParser::ModParser(bool isTcp):isTcp(isTcp){
  start(false);//false: for master, better than trash.
}

bool ModParser ::isWrite(void)const{
  return ::isWrite(functionCode);
}
/** response is an error reporting packet*/
bool ModParser ::reportsError(void)const{
  return ::reportsError(functionCode);
}

void ModParser ::start(bool asRequest){
  this->asRequest=asRequest;
  state=isTcp?mbrTransactionId:mbrLuno;
  errorCode=mb_OK;
  expectLow=false;
  info.forget();//#4debug
  byteCount=0;
  bytesProcessed=0;
}

u16 ModParser ::hilo(u8 incoming){
  return previous << 8 | incoming;
}

bool ModParser::lowByte(u16 &word,u8 incoming){
  if(expectLow) {
    expectLow = false;
    word= hilo(incoming);
  } else {
    expectLow = true;
  }
  return !expectLow;
}

bool ModParser::isComplete(void){
  if(isTcp&&state==mbrCrc){
    state=mbrComplete;
  }
  return state==mbrComplete;
}
/**call this when isComplete triggers a crc check and that fails.*/
void ModParser ::stop(void){
  state=mbrCorrupt;
  expectLow = false;
}

/**return whether packet is complete, doesn't include checking crc but does include receiving it*/
bool ModParser ::parse(u8 incoming){
  ++bytesProcessed;
  switch(state) {
  case mbrCorrupt:
    --bytesProcessed;//do nothing, except undo debug stat.
    break;
  case mbrErrorCode:
    state = mbrCrc;
    errorCode=ModErrorCode(incoming);
    break;
  case mbrComplete:
    break;
  case mbrTransactionId: //tcp header first word
    if(lowByte(transactionId,incoming)){
      state = mbrProtocolId;
    }
    break;
  case  mbrProtocolId: //tcp header fixed second word
    if(lowByte(crcsum,incoming)){//abusing crcsum for protocolId
      if(crcsum!=0){
        state = mbrCorrupt;
      } else {
        state = mbrPduLength;
      }
    }
    break;
  case mbrPduLength:  //tcp header additional length field.
    if(lowByte(tcpLength,incoming)){
      state = mbrLuno;
    }
    break;
  case mbrLuno:
    info.luno= incoming;
    state = mbrFunction;
    break;
  case mbrFunction:
    functionCode = ModFunction(incoming);
    if(asRequest) {//requests always have address/length stanza
      state = mbrAddress;
    } else {
      //todo:2 handle unsupported function codes
      if(reportsError()) {
        state = mbrErrorCode; //expecting one byte of detail
      } else if(isWrite()) { //expect address+quantity+crc
        state = mbrAddress;
      } else {
        state = mbrByteCount;
      }
    }
    break;
  case mbrAddress:
    if(lowByte(info.address,incoming)){
      state = mbrQty;
    }
    break;
  case mbrQty:
    if(lowByte(info.length,incoming)){
      //only received items are parsed, if a request then we are a slave and a write has data, if not a request then we are a master and reads have data.
      if(asRequest==isWrite()){
        state = mbrByteCount;//1st byte of payload
      } else {
        state = mbrCrc;
      }
    }
    break;
  case mbrByteCount:
    if(info.lengthCode() == incoming) {//this presumes registers, not coils.
      byteCount=incoming;
      state =  byteCount ? mbrBody : mbrCrc;//COA, count should never be 0 for a useful packet.
    } else {
      state = mbrCorrupt; //so bad we can't find the end of the packet.
    }
    break;
  case mbrBody:
    if(-- byteCount== 0) {
      state = isTcp?mbrComplete: mbrCrc;
    }
    break;
  case mbrCrc:
    if(lowByte(crcsum,incoming)){
      state = mbrComplete;
    }
    break;
  } //end switch
  previous=incoming;
  return isComplete();
} //end incremental parse

