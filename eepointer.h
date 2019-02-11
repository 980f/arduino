#pragma once  //(C)2019 Andy Heilveil, github/980F

#include <EEPROM.h>
struct EEreference {
  const uint16_t addr;//address for EEPROM calls
  EEreference(unsigned addr): addr(addr) {}
  
  operator char () {
    return EEPROM.read(addr);
  }
  
  void operator=(char value) {
    EEPROM.write(addr, value);
  }
};

/** both hasNext and next interfaces as well as *ptr++ */
class EEPointer {
    uint16_t eeaddress;//address for EEPROM calls
  public:
    EEPointer(unsigned startaddress = 0): eeaddress(startaddress) {
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

    EEreference operator *() {
      return EEreference(eeaddress);
    }

    EEPointer operator ++() {
      EEPointer copy = *this;
      ++eeaddress;
      return copy;
    }

    /** address of next cell */
    uint16_t location() {
      return eeaddress;
    }

};
