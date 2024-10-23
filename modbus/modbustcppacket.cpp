#include "modbustcppacket.h"
#include "logger.h"

void ModbusTcpPacket::preamble(const ModbusBlock &block,bool includeAddress,bool startBlock){
  writer.hilo(transactionId);//transaction ID
  writer.hilo(0);//protocol ID
  u16 tcpLength(1+1);//start with 1 for the luno, 1 for function code
  if(includeAddress){
    tcpLength+=4;
  }
  if(startBlock){
    tcpLength+=block.lengthCode()+1; //num bytes in body plus byte for that length
  }
  writer.hilo(tcpLength);
}

ModPDU::PacketQuality ModbusTcpPacket::parsePreamble(){
  if(reader.stillHas(6)){//6 is sizeof our take on the mbap header
    transactionId=reader.getU16();
    if(reader.getU16()!=0){//protocol ID
      return Corrupt;
    }
    u16 needed=reader.getU16();//length of following
    if(!reader.stillHas(needed)){
      return TooShort;
    }
    return WellFormed;
  } else {
    return TooShort;
  }
}

ModbusTcpPacket::ModbusTcpPacket():ModPDU(buffer,sizeof(buffer)){
  //#nada
  transactionId=0;
}

void ModbusTcpPacket::slide(){
  int amount=reader.used();//amount parsed
  reader.remove(amount);//should become a smaller, empty reader.
  writer.rewind(amount);
}

void ModbusTcpPacket::prepareForReception(){
  writer.rewind();//should be whole allocation
}
