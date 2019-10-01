#pragma once //(C)2019 Andy Heilveil, github/980F

#include "chainprinter.h"

class TwinConsole {
  public:
    ChainPrinter usb;
    ChainPrinter uart;
    TwinConsole(): usb(Serial), uart(Serial1) {
      //#done unless we find we can call begin here.
    }

    /** @returns keystrokes from every source, randomly interleaved, 0 if there are no strokes present. */
    unsigned getKey() {
      if (Serial && Serial.available()) {
        return Serial.read();
      }
      if (Serial1.available()) {
        return Serial1.read();
      }
      return 0;
    }

    //sets baud rate of real uart, also enables the USB one.
    void begin(unsigned long baudrate=500000) {
      Serial.begin(500000);//number here doesn't matter.
      Serial1.begin(baudrate);//hardware serial. up the baud to reduce overhead.
    }


    //print a series of items on both serial ports
    template<typename ... Args> TwinConsole& operator()(const Args ... args) {
      if (sizeof... (args)) {
        if (Serial) usb(args...);
        uart(args...);//uarts always are ready
      }
      return *this;
    }

};
