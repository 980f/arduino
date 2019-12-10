#pragma once //(C) 2019 Andy Heilveil, github/980f
#include <Print.h>

/* todo:
  ARDUINO_ARCH_SAMD

  ARDUINO_ARCH_SAM ARDUINO_SAM_DUE __SAM3X8E__
*/
#ifdef ARDUINO_ARCH_AVR
#define HaveEEPrinter 1
#include <EEPROM.h>

class EEPrinter : public Print {
    unsigned ptr;

  public:
    explicit EEPrinter(int start): ptr(start) {}

    size_t write(uint8_t data) override {
      //    	dbg(ptr,':',data,' ',char(data));
      EEPROM.write(ptr++, char(data));//#without the char cast data was corrupted. Someday we will take it out and figure out why.
      return 1;
    };

    int availableForWrite()  {
      return EEPROM.length() - ptr;
    }
};

#else
#warning no EEPrinter for your architecture

class EEPrinter : public Print {
  public:
    explicit EEPrinter(int ) {}
    size_t write(uint8_t data) override {
      return 0;
    };

    int availableForWrite()  {
      return 0;
    }
};

#endif //arch_avr
