#pragma once //(C) 2019 Andy Heilveil, github/980f

#include "wirewrapper.h"

class EDSir : public WireWrapper {
WireWrapper ww;
WIred<char> irchar; 

  EDSir(uint8_t address=0x60):
    ww(address),
    irchar(ww, 0) //port 0 is asciified map of EDS controller
  {
  }

  char key(){
    return irchar.fetch(); //I2C read, 0.1 ms\
  }

  bool isPresent(){
  	return ww.isPresent();
  }

}
