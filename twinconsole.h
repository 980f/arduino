#pragma once //(C)2019 Andy Heilveil, github/980F

#include "chainprinter.h"

class TwinConsole {
  public:
    ChainPrinter usb;
    ChainPrinter uart;
    TwinConsole(): usb(Serial), uart(Serial1) {
      //#done unless we find we can call begin here.
    }

    /** @returns keystrokes from every source, randomly interleaved, -1 if there are no strokes present. */
    int getKey() {
      if (Serial && Serial.available()) {
        return Serial.read();
      }
      if (Serial1.available()) {
        return Serial1.read();
      }
      return -1;
    }

    void begin() {
      Serial.begin(500000);//number here doesn't matter.
      Serial1.begin(500000);//hardware serial. up the baud to reduce overhead.
    }

    template<typename ... Args> TwinConsole& operator()(const Args ... args) {
      if (sizeof... (args)) {
        if (Serial) usb(args...);
        uart(args...);//uarts always are ready
      }
      return *this;
    }

};

