#include "modbus.h"

bool isWrite(ModFunction functionCode){
  switch(functionCode) {
  case mbfWriteMultipleRegisters:
  case mbfWriteMultipleCoils:
  case mbfWriteCoil:
  case mbfWriteRegister:
    return true;
  default:
    return false;
  }
} // isWrite

bool isRead(ModFunction functionCode){
  switch(functionCode) {
  case mbfReadCoils:
  case mbfReadHoldingRegisters:
  case mbfReadInputs:
    return true;
  default:
    return false;
  }
}

bool isReadOnly(ModFunction functionCode){
  switch(functionCode){
  case mbfReadInputs:
  case mbfReadInputRegisters:
  //case mbfReadHoldingRegisters: //separate 3x from 4x  -  #Not sure if necessary.
    return true;
  default:
    return false;
  }
}

bool isCoiled(ModFunction functionCode){
  switch(functionCode) {
  case mbfWriteMultipleCoils:
  case mbfReadCoils:
  case mbfWriteCoil:
  case mbfReadInputs: //this one item may be a bit risky, caller needs to check the function code to distinquish from actual coils.
    return true;
  default:
    return false;
  }
} // isCoiled

ModbusBlock::ModbusBlock(bool isCoils, int luno, bool isRO, unsigned addr, unsigned length): isCoils(isCoils), luno(luno), isRO(isRO), address(addr), length(length){
  //#nada
}

ModbusBlock::ModbusBlock(const ModbusBlock&other):
  isCoils(other.isCoils),
  luno(other.luno),
  isRO(other.isRO),
  address(other.address),
  length(other.length){
  //#nada
}

ModbusBlock&ModbusBlock::operator =(const ModbusBlock&rhs){
  isCoils = rhs.isCoils;
  luno = rhs.luno;
  address = rhs.address;
  length = rhs.length;
  return *this;
}

bool ModbusBlock::operator ==(const ModbusBlock&rhs) const{
  if(isCoils != rhs.isCoils) {
    return false;
  }
  bool exact = luno == rhs.luno && address == rhs.address && length == rhs.length;
  if(exact) {
    return true;
  }
  //use a highly unlikely key to indicate "match any address"
  if(rhs.length == 0 || length == 0) {
    return luno == rhs.luno;
  }
  return false;
} // ==

void ModbusBlock::forget(){
  //isCoils is left alone on purpose.
  luno = ~0;
  address = ~0;
  length = 0;
}

bool ModbusBlock::operator <(const ModbusBlock&rhs) const {
  //first mismatch term is last computed, the double | is essential here.
  int cmp = isCoils - rhs.isCoils || luno - rhs.luno || address - rhs.address || length - rhs.length;

  return cmp < 0;
}

u8 ModbusBlock::lengthCode() const {
  if(isCoils) {
    return (length + 7) / 8; //number of bytes from number of coils.
  } else {
    return length * 2;
  }
}

ModFunction ModbusBlock::functionCode(bool forWrite) const {
  if(forWrite) {
    if(isCoils) {
      if(isRO) {
        return mbfUnknown; //defective block
      } else {
        return mbfWriteMultipleCoils;
      }
    } else {
      if(isRO) {
        return mbfUnknown; //defective block
      } else {
        return mbfWriteMultipleRegisters;
      }
    }
  } else {
    if(isCoils) {
      if(isRO) {
        return mbfReadInputs;
      } else {
        return mbfReadCoils;
      }
    } else {
      if(isRO) {
        return mbfReadInputRegisters;
      } else {
        return mbfReadHoldingRegisters;
      }
    }
  }
} // functionCode

bool ModbusBlock::contains(u16 which) const {
  return which >= address && which < address + length;
}

bool ModbusBlock::handles(u16&which, unsigned sizeofOne) const {
  if(contains(which)) {
    which -= address;
    if(sizeofOne) {
      which /= sizeofOne;
    }
    return true;
  } else {
    return false;
  }
} // handles

bool ModbusBlock::contains(const ModbusBlock&block, bool ignoreLuno) const {
  if(ignoreLuno || (luno == block.luno)) { //same partition of device or don't care which (presumably many copies of some block)
    if(isRO == block.isRO){//# spread out for debug, could be done in a single AND
      if(isCoils == block.isCoils) {
        if(contains(block.address)) {
          if(contains(block.end() - 1)) {
            return true;
          }
        }
      }
    }
  }
  return false;
} // contains

u16 ModbusBlock::end() const {
  return address + length;
}

bool ModbusBlock::isSingle(u16 address) const {
  return length == 1 && address == this->address;
}

int ModbusBlock::bias(const ModbusBlock &parentBlock, int sizeofItem){
  //todo: idiot checks that return a negative number as an error code.
  return (this->address-parentBlock.address)/sizeofItem;
}

bool ModbusPointer::hasNext(){
  return length > 0;
}

//////////////////////////
u16 ModbusPointer::next(){
  if(length) {
    --length;
    return address++;
  } else {
    return address - 1; //usually non-fatal, caller should always if(hasNext()) before calling next().
  }
}

bool ModbusPointer::hasMore(){
  if(length) {
    --length;
    ++address;
    return true;
  } else {
    return false;
  }
}

ModbusPointer::ModbusPointer(const ModbusBlock&block): address(block.address), length(block.length){
  //#nada
}

//////////////
RegisterBlock::RegisterBlock(int luno, bool isRO, int addr, int length): ModbusBlock(false, luno, isRO, addr, length){
  //#nada
}

RegisterBlock::RegisterBlock(const RegisterBlock&other): ModbusBlock(other){
  //#nada
}

bool RegisterBlock::isSingle32(u16 address){
  return length == 2 && address == this->address;
}

//////////////
CoilBlock::CoilBlock(int luno, bool isRO, unsigned addr, unsigned length): ModbusBlock(true, luno, isRO, addr, length){
  //#nada
}

CoilBlock::CoilBlock(const CoilBlock&other): ModbusBlock(other){
  //#nada
}

/////////////
ModbusBlock::Indexer::Indexer(const ModbusBlock&block, int sizeofItem):
  block(block),
  offset(0),
  sizeofItem(sizeofItem){
  //#nada
}

bool ModbusBlock::Indexer::hasNext(){
  return offset + sizeofItem <= block.length;
}

u16 ModbusBlock::Indexer::address(){
  return offset + block.address;
}

int ModbusBlock::Indexer::next(){
  offset += sizeofItem;
  return (offset / sizeofItem) - 1;
}
