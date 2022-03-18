#pragma once //(C)2019 Andy Heilveil, github/980F


/** Tee's output to two destinations (Serial and Serial1), merges input from them as if from a single source. 
todo: make chainprinter's 'stifled' and autofeed stacker accessible. They were added after this class was written.
*/

#include "chainprinter.h"
#ifdef HAVE_HWSERIAL1
class TwinConsole: public Print {
  public:
    ChainPrinter usb;
    ChainPrinter uart;
    TwinConsole(): usb(Serial), uart(Serial1) {
      //#done unless we find we can call begin here.
    }

    /** @returns keystrokes from every source, randomly interleaved, 0 if there are no strokes present, nulls will get ignored. */
    byte getKey() {
      if (Serial && Serial.available()) {
        return Serial.read();
      }
      if (Serial1.available()) {
        return Serial1.read();
      }
      return 0;
    }

    /** sets baud rate of real uart, also enables the USB one. */
    void begin(uint32_t uartbaud = 115200) {
      Serial.begin(uartbaud);//number here doesn't actually matter.
      Serial1.begin(uartbaud);//hardware serial. up the baud to reduce overhead.
    }


    /** @returns this after printing a series of items */
    template<typename ... Args> TwinConsole& operator()(const Args ... args) {
      if (sizeof... (args)) {
        if (Serial) usb(args...);
        uart(args...);//real uarts always are ready (not really, data can get lost, but our printer doesn't allow for that)
      }
      return *this;
    }

    /**satisfy Print interface*/
    size_t write(byte value) override {
      if (Serial) usb.raw.write(value);
      return uart.raw.write(value);
    }

};
#else
#warning "twinconsole not supported on your selected processor, creating dummy version" 
class TwinConsole: public Print {
  public:
    ChainPrinter usb;
    TwinConsole(): usb(Serial) {
      //#done unless we find we can call begin here.
    }

    /** @returns keystrokes from every source, randomly interleaved, 0 if there are no strokes present, nulls will get ignored. */
    byte getKey() {
      if (Serial && Serial.available()) {
        return Serial.read();
      }
      return ~0;
    }

    void begin(uint32_t uartbaud = 115200) {
      Serial.begin(115200);
    }

    /** @returns this after printing a series of items */
    template<typename ... Args> TwinConsole& operator()(const Args ... args) {
      if (sizeof... (args)) {
        if (Serial) usb(args...);
      }
      return *this;
    }

    /** satisfy Print interface */
    size_t write(byte value) override {
      return Serial? usb.raw.write(value): 0;
    }

};

#endif

//marker for codespace strings, with newline prefixed. Without this or the Arduino provided F() constr strings take up ram as well as rom.
#define FF(farg)  F( "\n" farg)
