#include "modbusagent.h"
#include "logger.h"
#include "cheapTricks.h"

ModbusAgent::ModbusAgent( bool asSlave):
  asSlave(asSlave),
  lastSent(),
  received(),
  parser(true) //true: only supporting tcp at the moment.
{
  received.beginReception();//4 debug, one would hope the communications link would also call this at an appropriate time.
}

BigEndianer & ModbusAgent::beginRequest(ModbusTcpPacket &packet,const ModbusBlock &block, bool forWrite){
  parser.start(false);//forget last parse
  parser.info=block;//store block info because read responses don't tell you what their content is (while we could better interlock request and response we would then have to parse the command we sent).
  return packet.beginRequest(block,forWrite);
}

bool ModbusAgent::incrementalReception(u8 incoming){
  if(received.accept(incoming)){
    //try parsing to see if we have a full packet
    ModPDU::PacketQuality quality=received.verifyAndParse(parser);//points received.reader to datablock
    switch(quality){
    case ModPDU::TooShort: //haven't found end of packet yet.
      break;
    case ModPDU::WellFormed:
      if(processReceived()){
        //if master then we should have acted upon it already.
        //if slave then lastSent should have been updated and 'replyPending' set.
      } else {
        //here we can put some generic error handler.
      }
      break;

    default: //trashed. attempt some form of recovery
      received.prepareForReception();
      break;
    }
    return true;
  } else {
    //we have overflowed our buffer,
    //maydo: start stripping bytes off of the front to see if there is a packet lurking (if master)
    received.prepareForReception();
    return false;
  }
}

/////////////
ModbusSlave::ModbusSlave():ModbusAgent(false),
  replyPending(false){
  //#nada
}

ModbusSlave::~ModbusSlave(){
  //#nada
}

bool ModbusSlave::processReceived(){
  lastSent.transactionId=received.transactionId;
  bool isWrite(parser.isWrite());//FUE
  lastSent.beginResponse(parser.info,isWrite);
  ModErrorCode errcode=isWrite?receive(parser.info,received.reader):send(parser.info,lastSent.writer);
  if(errcode!=mb_OK){
    lastSent.errorResponse(errcode,parser.info,isWrite);
  }
  lastSent.reader.grab(lastSent.writer);
  replyPending=true;
  received.prepareForReception();
  return true;//we no longer call this function if the packet isn't already deemed cool
}
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////

ModbusMaster::ModbusMaster(int timeoutMillis):ModbusAgent(false),
  waitingForResponse(false),
  responseTimer(timeoutMillis,MyHandler(ModbusMaster::timedOut))
{
}

ModbusMaster::~ModbusMaster(){
  //#nada
}

void ModbusMaster::timedOut(){
  if(flagged(waitingForResponse)){//todo:M replace use of 'flagged' with an AtomicBool class.
    ++stats.timeouts;
  }
  //do NOT resend last command as it may be defective in a fashion that would cause infinite retries of just it.
  //just proceed as if transaction was not in progress, or even wait until next poll.
  //might do a reconnect if these are rare but sticky.
}

bool ModbusMaster::processReceived(){
  responseTimer.cancel();
  if(received.transactionId!=lastSent.transactionId){
    if(tcpError()){
      ++stats.badResponses;
      return false;
    }
  }
  if(parser.reportsError()){
    parseError();
    ++stats.badResponses;
    return false;
  } else {
    ++stats.goodResponses;
    if(parser.isWrite()){
      //nothing to do here. a response to a write is no more than an ack. We could perhaps trigger the next poll sooner, but why bother?
    } else {
      if(parser.info.isCoils){
        u16 coil=parser.info.address;
        ModPDU::CoilPointer coiler(received.reader);//todo: make sure is is pointing at datablock
        for(int count=parser.info.length; count -->0;){
          setCoil(coil++,coiler.next());
        }
      } else {
        u16 reg=parser.info.address;//#for 1214 we expected to have a 0x10 here, but are getting a zero.
        BigEndianer &reader(received.reader);//todo: make sure is is pointing at datablock
        for(int count=parser.info.length; count -->0;){
          switch(registerType(reg)){
          case 0: //bad address
            wtf("Bad Register readback: %04X",reg++);
            break;
          case 1: //typical register
            setRegister(reg++,reader.getU16());
            break;
          case 2: //32bit integer
            set32(reg++,reader.getu32());
            --count;
            ++reg;
            break;
          case 3: //32bit ieee
            setFloat(reg++,reader.getFloat());
            --count;
            ++reg;
            break;
          }
        }
      }
    }
  }
  return true;
}

bool ModbusAgent::inGroup(u16 &which, u16 groupbase, int groupLength){
  if(which>=groupbase&&which<groupbase+groupLength){
    which -=groupbase;
    return true;
  } else {
    return false;
  }
}

int ModbusMaster::setCoil(u16 /*which*/, bool /*on*/){
  return mb_ILLEGAL_DATA_ADDRESS;
}

int ModbusMaster::registerType(u16 /*which*/){
  return 0;
}

int ModbusMaster::setRegister(u16 /*which*/, u16 /*value*/){
  return mb_ILLEGAL_DATA_ADDRESS;
}

int ModbusMaster::set32(u16 /*which*/, u32 /*value*/){
  return mb_ILLEGAL_DATA_ADDRESS;
}

int ModbusMaster::setFloat(u16 /*which*/, float /*value*/){
  return mb_ILLEGAL_DATA_ADDRESS;
}


void ModbusMaster::CommStatistics::clear(){
  timeouts=0;
  goodResponses=0;
  badResponses=0;
}

ModbusMaster::CommStatistics::CommStatistics(){
  clear();
}
