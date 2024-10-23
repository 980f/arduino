#include "modpdu.h"

ModPDU::ModPDU(u8 *buffer, int bufferSize):
  writer(buffer, bufferSize),
  reader(buffer, bufferSize){
  //extension must init reader and writer
}

void ModPDU::preamble(const ModbusBlock &/*block*/, bool /*includeAddress*/, bool /*startBlock*/){
  //#nada
}

bool ModPDU::stillHave(int body){
  return reader.stillHas(body+postamble());
}


void ModPDU::beginReception(void){
  writer.rewind();
  isComplete=false;
}

bool ModPDU::accept(u8 incoming){
  bool overran=!writer.hasNext();
  writer.next() = incoming; //add to buffer regardless of whether byte is any good
  return !overran;
} /* accept */


/** @return whether incoming is the same as the last outgoing, i.e. we see an echo.*/
bool ModPDU::verifyEcho(u8 incoming){
  return writer.hasNext()&&writer.next() == incoming;
}

BigEndianer & ModPDU::formatPacket(const ModbusBlock&block, bool forWrite, bool includeAddress, bool startBlock){
  isComplete = false;
  writer.rewind();
  preamble(block,includeAddress,startBlock);//tcp
  writer.next() = block.luno;

  functionCode = block.functionCode(forWrite);
  writer.next() = u8(functionCode);
  if(includeAddress) {
    writer.hilo(block.address);
    writer.hilo(block.length);
  }
  if(startBlock) {
    writer.next() = block.lengthCode(); //num data bytes
  }
  //we don't add rtu checksum here, we are slicing this code out of an older class.
  return writer;
} /* formatPacket */

/* @return pointer ready for adding data words.*/
BigEndianer & ModPDU::beginRequest(const ModbusBlock&block, bool forWrite){
  return formatPacket(block, forWrite, true, forWrite);
}

/** @return pointer ready for adding data words.*/
BigEndianer & ModPDU::beginResponse(const ModbusBlock&block, bool forWrite){
  return formatPacket(block, forWrite, forWrite, !forWrite);
}

BigEndianer &ModPDU::errorResponse(ModErrorCode error, const ModbusBlock &block, bool forWrite){
  isComplete = false;
  writer.rewind();
  preamble(block,false,false);
  writer.next() = block.luno;;

  functionCode = block.functionCode(forWrite);
  writer.next() = u8(functionCode|0x80);
  writer.next()=error;
  isComplete = true;
  return writer;
}

BigEndianer & ModPDU::respond(const ModParser &parsed){
  //trust asRequest parser:
  writer.rewind();
  //todo:0 has to do something with preamble. this is rtu only code at the moment.
  writer.skip(2);//luno and function code
  if(parsed.isWrite()){
    writer.skip(4);//start and length (we only support multiple read/write at this point)
  } else {
    writer.next()=parsed.info.lengthCode(); //presume read request
    //above is byte count
  }
  return writer;
}


void ModPDU::issuePacket(bool asRequest){
  isRequest = asRequest;
  reader.snap(writer);
  writer.rewind(); //to receive echo and response
}

ModPDU::PacketQuality ModPDU::verifyError(PacketQuality theError){
  if(theError!=WellFormed){
    return theError;//# broken into an if-else for sake of breakpoint on error.
  } else {
    return WellFormed;
  }
}


ModPDU::PacketQuality ModPDU::verifyBlock(bool rollbackHeader){
  if(stillHave(1)) {//length
    int length = reader.next();
    if(stillHave(length)) {
      reader.skip(length);
      if(checkCRC()) {
        return verifyError(Corrupt); //bad crc
      }
      reader.rewind(length + postamble()+ (rollbackHeader ? 4 : 0)); //point to starting address of DATA  in block. In revs before tcp additions we pointed to the length byte.
      return WellFormed;
    } else {
      return verifyError(TooShort); //short packet
    }
  } else {
    return verifyError(TooShort); //really short packet
  }
} /* verifyBlock */

ModPDU::PacketQuality ModPDU::verifyAfter(unsigned int skip, unsigned int backMore ){
  if(stillHave(skip)) {
    reader.skip(skip);
    if(checkCRC()) {
      return verifyError(Corrupt); //bad crc
    }
    reader.rewind(postamble() + skip + backMore); //point to starting address
    return WellFormed; //well formed error packet

  } else {
    return verifyError(TooShort);
  }
} /* verifyAfter */

ModPDU::PacketQuality ModPDU::verifyAndParse(ModParser&parser){
  bool isRequest=parser.asRequest;
  reader.snap(writer);
  if(!reader.hasNext()) {
    return verifyError(Empty);
  }

  PacketQuality mbapIfTcp=parsePreamble();
  if(mbapIfTcp!=WellFormed){
    return verifyError(mbapIfTcp);
  }
  //todo:1 the doubling of functionCode/info.functionCode implies some cleanup is possible: the issue here is using a single packet for both read and write.
  parser.info.luno = reader.next();
  if(reader.hasNext()) {
    functionCode=parser.functionCode= ModFunction(reader.next());
    parser.info.isCoils = isCoiled(parser.functionCode);
    parser.info.isRO = isReadOnly(parser.functionCode);
    if(isRequest) { //if slave: received this, master: checking echo
      if(stillHave(4)) {
        parser.info.address=reader.getU16();
        parser.info.length=reader.getU16();
        if(/*parser.info.*/isWrite()) {
          return verifyBlock(false); //point to data block's length
        } else {
          return verifyAfter(0, 0);
        }
      } else {
        return verifyError(TooShort);
      }
    } else {//isReply
      if(parser.reportsError()) { //looks like an error response
        parser.errorCode=ModErrorCode(reader.next());
        return verifyAfter(0, 0);
      } else { //ok packet (so far :)
        if(parser.isWrite()) {
          parser.info.address=reader.getU16();
          parser.info.length=reader.getU16();
          return verifyAfter(0, 0);
        } else { //presume multiple read:
          return verifyBlock(false); //point to size byte.
        }
      }
    }
  } else {
    return verifyError(TooShort);
  }
}

///////////

ModPDU::CoilPointer::CoilPointer(Indexer<u8> &buffer):
  buffer(buffer),
  peek(0),
  bitIndex(8)//init bitIndex to 'byte exhausted'
{
  //#nada
}

void ModPDU::CoilPointer::operator =(const ModPDU::CoilPointer other) {
  buffer = other.buffer;
  peek = other.peek;
  bitIndex = other.bitIndex;
}

bool ModPDU::CoilPointer::next(int howMany){
  for(int i = 0; i < howMany; ++i) {
    if(!peek||bitIndex==8){//peek term is a COA
      peek=&buffer.next();
      bitIndex=0;
    }
  }
  return bit(*peek,bitIndex++);
}

bool ModPDU::CoilPointer::hasNext(){
  if(bitIndex==8){
    return buffer.hasNext();
  } else {
    return peek!=nullptr;//COA, should always be true here
  }
}

void ModPDU::CoilPointer::pack(bool coil){
  if(!peek||bitIndex==8){//peek term is a COA
    peek=&buffer.next();
    bitIndex=0;
    *peek=0;//buffer is getting reused by callers without clearing so we clean it up here.
  }
  *peek |= coil<<bitIndex++;
}

