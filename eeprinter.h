#pragma once //(C) 2019 Andy Heilveil, github/980f
#include <Print.h>

#include <EEPROM.h>

/* todo:
  ARDUINO_ARCH_SAMD

  ARDUINO_ARCH_SAM ARDUINO_SAM_DUE __SAM3X8E__
*/
#ifdef ARDUINO_ARCH_AVR
#define HaveEEPrinter 1

class EEPrinter : public Print {
    unsigned ptr;

  public:
    explicit EEPrinter(int start): ptr(start) {}
    
    size_t write(uint8_t data) override {
//    	dbg(ptr,':',data,' ',char(data));
      EEPROM.write(ptr++, char(data));//#without the char cast data was corrupted. Someday we will take it out and figure out why.
      return 1;
    };

    int availableForWrite() override {
      return EEPROM.length() - ptr;
    }
};

#else
#error no EEPrinter for your architecture
#endif //arch_avr
