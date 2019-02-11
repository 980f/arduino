#pragma once  //(C)2019 Andy Heilveil, github/980F

#include <EEPROM.h>

/** both hasNext and next interfaces as well as *ptr++ */
class EEStream {
    uint16_t eeaddress;//address for EEPROM calls
  public:
    EEStream(unsigned startaddress = 0): eeaddress(startaddress) {
      //#done
    }
    //////////////////// javaish interface
    bool hasNext()const {
      return eeaddress < EEPROM.length();
    }

    char next() {
      return EEPROM.read(eeaddress++);
    }

    ////////////////// pointer like interface
    operator bool()const {
      return hasNext();
    }

    char operator *() {
      return EEPROM.read(eeaddress);
    }

    EEStream operator ++() {
    	EEStream copy=*this;
      ++eeaddress;
      return copy;
    }

    /** This takes 3.3 ms */
    EEStream &operator ()(char byte) {
      EEPROM.write(eeaddress, byte);
      return *this;
    }

    /** address of next cell */
    uint16_t location() {
      return eeaddress;
    }

};
