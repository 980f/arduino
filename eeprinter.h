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
      EEPROM.write(ptr++, data);
    };

    int availableForWrite() override {
      return EEPROM.length() - ptr;
    }
    //  virtual void flush() {  }
};

#else
#error no EEPrinter for your architecture
#endif //arch_avr
