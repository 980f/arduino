#pragma once //(C) 2019 Andy Heilveil, github/980f

#include "wirewrapper.h"

class EDSir: public WireWrapper {
    WIred<char> irchar;
  public:
    //port 0 is asciified map of EDS controller
    EDSir(uint8_t address = 0x60): WireWrapper(address), irchar(*this, 0) {
      //#done.
    }
    //may make this an operator char()
    char key() {
      return irchar.fetch(); //I2C read, 0.1 ms
    }
};
