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

    byte next() {
      return EEPROM.read(eeaddress++);
    }

    ////////////////// pointer like interface
    operator bool() const {
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

    /** crafted to satisfy Print but we don't make this class extend Print so as to not create a vtable is this method isn't used in that fashion.
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

/** extend eep address manager into a formatted eewriter. Compared to a EEPointer this guy has a vtable and the state storage of Print. Altogether that is about 6 bytes of ram, and maybe 30 of extra code.   */
struct EEPrinter : public EEPointer, public Print {
  EEPrinter(unsigned addr=~0): EEPointer(addr) {}
  size_t write(byte value) override {
    return EEPointer::write(value);
  }
};
