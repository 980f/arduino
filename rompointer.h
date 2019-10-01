#pragma once //(C) Andy Heilveil, github/980f
/** pointer for reading from PROGMEM
*/

using RomAddr = unsigned;//const PROGMEM char *;

struct RomPointer  {
  RomAddr addr;
  RomPointer(RomAddr addr): addr(addr) {}

  char operator *() {
    return pgm_read_byte(addr);
  };

  RomPointer operator ++(int) {//# do NOT return a reference
    RomPointer copyme = *this;
    ++addr;
    return copyme;
  }

  int operator -( RomAddr other) {
    return addr - other;
  }

};

