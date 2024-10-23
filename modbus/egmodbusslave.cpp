#include "egmodbusslave.h"


enum EgRegs {
  LayoutVersion,
  BuildId,
  Echoer,
  WordOrder,
  WordOrderis32bits,
  FpFormatIndicator,
  FpFormatIndicatoris32bits,
  HeadQuantity,

  NumEgRegs //count of registers.
};

const RegisterBlock egRegsBlock(0,true,0,NumEgRegs);

//gauges moved from system to per head.

ModErrorCode NexModbusSlave::getRegisters(ModbusPointer &pointer, BigEndianer &packet){
  while(pointer.hasNext()){
    switch(pointer.next()){
    case ModbusLayoutVersion:
      packet.hilo(ModbusLayoutVersionNumber);
      break;
    case BuildId:
      packet.insertVersion();//presumes BigEndianer is included source, not independently maintained library.
      break;
    case Echoer:
      if(echoregister){
        echoCache++;
      } else {
        echoCache=0;
      }
      packet.hilo(echoCache);
      break;
    case WordOrder:
      //the hasMore below bumps the pointer, to compensate for the 32 bit value.
      if(pointer.hasMore()){//don't let them read half of a 32 bit quantity
        packet.put(u32((1234<<16)+5678));//this weird expression is used to make sure we are doing this the same as a 32 bit int would be done elsewhere.
        break;
      } else {
        return mb_ILLEGAL_DATA_ADDRESS;
      }
    case FpFormatIndicator:
      if(pointer.hasMore()){//don't let them read half of a 32 bit quantity
        packet.put(12.34);
        break;
      } else {
        return mb_ILLEGAL_DATA_ADDRESS;
      }
    case HeadQuantity:
      packet.hilo(nex.heads.quantity());
      break;
    default:
      return mb_ILLEGAL_DATA_ADDRESS;
    }
  }
  return mb_OK;
}

ModErrorCode NexModbusSlave::setRegister(unsigned &address,BigEndianer &packet){
  switch(address++){
  case ModbusLayoutVersion:
    hostVersion= packet.getU16();
    return mb_OK;
  case BuildId:
    //ro
    break;
  case Echoer:
    echoregister=packet.getU16();
    return mb_OK;
  case WordOrder:
    break;
  case FpFormatIndicator:
    break;
  case HeadQuantity:
    break;
  }
  return mb_ILLEGAL_DATA_ADDRESS;
}


const RegisterBlock floatingResults(-1,true,32,14); //leaving space for other stuff here,


const CoilBlock headContactBits(-1,false,0,NumNexOlCoils); //write bits

//const CoilBlock headGaugeStatuses(-1,true,16,7/*number of supported analog inputs*/);
//const CoilBlock headProcessStatuses(-1,true,32,0/*dynamically manipulated on use*/);

static void headCoilReader(Head &head,const ModbusBlock &block, BigEndianer &packet){
  ModPDU::CoilPointer coils(packet);
  unsigned address(block.address-headStatusBits.address);//we only call this routine for headStatusBits requests.
  for(int num=block.length;num-->0;){
    coils.pack(head.indicator(address++));
  }
}

static void headContactReader(Head &head,const ModbusBlock &block, BigEndianer &packet){
  ModPDU::CoilPointer coils(packet);
  unsigned address(block.address-headContactBits.address);//we only call this routine for headContactBits requests.
  for(int num=block.length;num-->0;){
    coils.pack(head.getControl(address++));
  }
}

void NexModbusSlave::headCoilWriter(Head &head,const ModbusBlock &block, BigEndianer &packet){
  ModPDU::CoilPointer coils(packet);
  int which = block.address - headContactBits.address;
  while(coils.hasNext()) {
    bool active = coils.next();
    head.setControl(NexHeadControls(which++),active);
  }
}

void NexModbusSlave::systemCoilReader(const ModbusBlock &block, BigEndianer &packet){
  ModPDU::CoilPointer coils(packet);
  unsigned address(block.address-nexSysCoilsBlock.address);//we only call this routine for headStatusBits requests.
  for(int num=block.length;num-->0;){
    coils.pack(systemCoilGetter(address++));
  }
}


void NexModbusSlave::systemInputReader(const ModbusBlock &block, BigEndianer &packet){
  ModPDU::CoilPointer coils(packet);
  unsigned address(block.address-nexSysInputsBlock.address);//we only call this routine for headStatusBits requests.
  for(int num=block.length;num-->0;){
    coils.pack(systemCoilGetter(address++));
  }
}

///////////////////////////////
NexModbusSlave::NexModbusSlave(NexSystem *nex):
  nex(*nex),
  echoregister(0),  //default with off
  hostVersion(ModbusLayoutVersionNumber){//if host doesn't inform us of its protocol then we act like it is happy with our latest.
  //#nada
}

NexModbusSlave::~NexModbusSlave(){
  //#nada
}

int NexModbusSlave::selectedApp(Head &head){
  return head.applicationRC.appIds.ordinalOf(head.apps.getCurrentRec()) + 1;
}

/* responding to a write request */
ModErrorCode NexModbusSlave::receive(const ModbusBlock &block, BigEndianer &packet){
  //there were dregs of code here that tried to implement a system suppressRemote, but we have no specification for that.
  if(Head *headp=getHead(block.luno)){//then is per-head group
    Head &head(*headp);
    //floatingResults.length = 2* head.numAnalytes();
    if(head.stateinfo.suppressRemote.isLocked()) {
      return mb_SLAVE_DEVICE_BUSY;
    } else if(headContactBits.contains(block,true/*ignoreLuno*/)){
      headCoilWriter(head,block,packet);
      return mb_OK;
    }
    if(headStatusBits.contains(block,true/*ignoreLuno*/)){  //to ask andy:  do we need to check for this? -> to report to customers what they are doing wrong.
      dbg("MB: tried to write to status bits");
      return mb_ILLEGAL_DATA_ADDRESS;  //read only block
    } else if(scaledResults.contains(block, true)) {
      dbg("MB: tried to write to analytical results, scaled integer format");
      return mb_ILLEGAL_DATA_ADDRESS;  //read only block
    } else if(floatingResults.contains(block, true)) {
      dbg("MB: tried to write to analytical results, FP format");
      return mb_ILLEGAL_DATA_ADDRESS;  //read only block
    } else if(appSelectControl.contains(block,true/*ignoreLuno*/)){
      for(ModbusBlock::Indexer looper(block);looper.hasNext();) {        
        nexSys->online.analogInput(looper.next(),double(packet.getU16())/65536.0);
      }
      return mb_OK;
    } else {
      //we ignore our block definitions for our readonly blocks, folding them in with totally bogus blocks.
      dbg("MB: tried to write to unimplemented address");
      return mb_ILLEGAL_DATA_ADDRESS;
    }
  } else if(block.luno==0) {//system stuff
    if(nexSysRegsBlock.contains(block)){
      //only one register is writable:
      if(block.address==Echoer&&block.length==1){
        echoregister=packet.getU16();
        return mb_OK;
      } else {
        return mb_ILLEGAL_DATA_ADDRESS;
      }
    } else if(nexSysCoilsBlock.contains(block)){
      return systemCoilSetter(block,packet);
    } else {
      return mb_ILLEGAL_DATA_ADDRESS;
    }
  } else {
    return mb_GATEWAY_TARGET_DEVICE_NOT_RESPONDING;
  }
}

static bool checkNumAnalytes = false;

/* responding to a read request */
ModErrorCode NexModbusSlave::send(const ModbusBlock &block, BigEndianer &packet){
  Head *ahead=getHead(block.luno);
  if(ahead){//then is per-head group
    Head &head(*ahead);
    floatingResults.length = 2* head.numAnalytes();

    if(headStatusBits.contains(block,true/*ignoreLuno*/)){
      headCoilReader(head,block,packet);
      return mb_OK;
    } else if(headContactBits.contains(block,true/*ignoreLuno*/)){
      headContactReader(head,block,packet);
    } else if(floatingResults.contains(block,true/*ignoreLuno*/)){//analyte results as floating point
      if(checkNumAnalytes && floatingResults.length > 2* head.numAnalytes()){
        return mb_ILLEGAL_DATA_ADDRESS;
      }
      if(block.length&1){//odd length is not legal
        return mb_ILLEGAL_DATA_ADDRESS;
      }
      for(ModbusBlock::Indexer looper(block,2);looper.hasNext();){
        packet.put(head.stateinfo.status.results[looper.next()]);
      }
      return mb_OK;
    } else if(scaledResults.contains(block,true/*ignoreLuno*/)){//analyte results scaled and clipped, there is a max here.
      if(checkNumAnalytes && scaledResults.length > head.numAnalytes()){
        return mb_ILLEGAL_DATA_ADDRESS;
      }
      for(ModbusBlock::Indexer looper(block);looper.hasNext();){
        packet.hilo(saturated(65535,head.fetchScaledResults(looper.next())));//analogOutput(0) is application select
      }
      return mb_OK;
    } else if(appSelectStatus.contains(block,true/*ignoreLuno*/)){
      auto *broom= head.reportData?&head.reportData->broom.last(): nullptr;
      for(ModbusBlock::Indexer looper(block);looper.hasNext();){
        switch(looper.next()){
        case ApplicationSelect:
          packet.hilo(head.modbusRecipeOutput());
          break;
        case WebSpeed:
          packet.hilo(broom ? broom->webSpeed : nex.online.modbusGaugeValue(WebSpeed));
          break;
        case WebDistance:
          packet.hilo(broom ? broom->position : nex.online.modbusGaugeValue(WebDistance));
          break;
        case SweepCount:
          packet.hilo(head.scanstate.pass); //todo:00 really should come from report broom.
          break;
        default:
          //todo: an error
          return mb_ILLEGAL_DATA_ADDRESS;
        }
      }
      return mb_OK;
    } else if(appSelectControl.contains(block,true/*ignoreLuno*/)){//we ignoreluno until we have gauges per head like we should have done from the beginning.
      for(ModbusBlock::Indexer looper(block);looper.hasNext();){
        nex.online.modbusGaugeValue(looper.next());
        /*
        case ApplicationSelect:
          packet.hilo(appSelectCache[block.luno]);
          break;
        case WebSpeed:
          packet.hilo(nex.online.modbusGaugeValue(FlowGaugeId));
          break;
        case WebDistance:
          packet.hilo(nex.online.modbusGaugeValue(OdometerId));
          break;
        default:
          //todo: an error
          return mb_ILLEGAL_DATA_ADDRESS;
        }*/
      }
      return mb_OK;
    } else {
      return mb_ILLEGAL_DATA_ADDRESS;
    }
  } else if(block.luno==0){
    if(nexSysRegsBlock.contains(block)){
      ModbusPointer looper(block);
      ModErrorCode err=getRegisters(looper,packet);
      return err;
    } else if(nexSysCoilsBlock.contains(block)){
      systemCoilReader(block, packet);
      return mb_OK;
    } else if(nexSysInputsBlock.contains(block)){
      systemCoilReader(block, packet);
      return mb_OK;
    } else {
      return mb_ILLEGAL_DATA_ADDRESS;
    }
  } else {
    return mb_ILLEGAL_DATA_ADDRESS;
  }
}
