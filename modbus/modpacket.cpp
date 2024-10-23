#include "modpacket.h"

#include "crc16m.h"

/** @returns xor of computed and received crc's, reader itself is moved past putative crc*/
u16 ModPacket::checkCRC(void){
  BigEndianer checker(reader);
  u16 check = Crc16m::compute(checker);
  return check ^ reader.getU16();
}

void ModPacket::addCrc(void){
  BigEndianer summer(writer);
  u16 sum = Crc16m::compute(summer);
  writer.hilo(sum); //at this point summer should point same as writer, but have "nothing left"
  isComplete = true;
}

ModPacket::ModPacket ():ModPDU(buffer,sizeof(buffer)){
  //#nada
}

void ModPacket::issuePacket(bool asRequest){
  addCrc();
  ModPDU::issuePacket(asRequest);
}

//end of file
