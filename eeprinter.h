#pramga once //(C) 2019 Andy Heilveil, github/980f
//eeprom allocations:
#include <Print.h>

#ifdef ARDUINO_ARCH_AVR
#include <EEPROM.h>
#if defined(ARDUINO_AVR_LEONARDO)
#define EESIZE 1024
#elif defined(ARDUINO_AVR_UNO)
#define EESIZE 1024
#elif defined(ARDUINO_AVR_MEGA)
#define EESIZE 4096
#else
#define EESIZE 512
#endif
/*
ARDUINO_ARCH_SAMD

ARDUINO_ARCH_AVR
*/
class EEPrinter : public Print {
    unsigned ptr;

  public:
    explicit EEPrinter(int start): ptr(start) {}
    size_t write(uint8_t data) override {      
      EEPROM.write(ptr++, data);    
    };

    int availableForWrite() override {
      return EESIZE - ptr;
    }
    //  virtual void flush() {  }
};

#else
#error no EEPrinter for your architecture
#endif //arch_avr
