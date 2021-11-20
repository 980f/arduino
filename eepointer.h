#pragma once  //(C)2019 Andy Heilveil, github/980F

#include <EEPROM.h>
struct EEreference {
  const uint16_t addr;//address for EEPROM calls
  EEreference(unsigned addr): addr(addr) {}

  operator byte () {
    return EEPROM.read(addr);
  }

  void operator=(byte value) {
    EEPROM.write(addr, value);
  }
};

/** both hasNext and next interfaces as well as *ptr++ */
class EEPointer {
  protected:
    uint16_t eeaddress;//address for EEPROM calls
  public:
    EEPointer(unsigned startaddress = ~0): eeaddress(startaddress) {
      //#done
    }
    //////////////////// javaish interface
    bool hasNext()const {
      return eeaddress < EEPROM.length();
    }

    EEreference next() {
      return EEreference(eeaddress++);
    }

    ////////////////// pointer like interface
    operator bool() const {
      return hasNext();
    }

    EEreference operator *() {
      return EEreference(eeaddress);
    }

    EEPointer operator ++(int) {
      EEPointer copy = *this;
      ++eeaddress;
      return copy;
    }

    /** address of next cell */
    uint16_t location() {
      return eeaddress;
    }

    /** crafted to satisfy Print but we don't make this class extend Print so as to not create a vtable if this method isn't used in that fashion.
        elsewhere we can extedn EEPointer to make it be a Print	*/
    size_t write(byte value) {
      if (hasNext()) {
        EEPROM.write(eeaddress++, value);
        return 1;
      } else {
        return 0;
      }
    }
};

//see eeprinter.h to replace what was once here

