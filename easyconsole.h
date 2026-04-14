#pragma once //(C)2019 Andy Heilveil, github/980F

#include "chainprinter.h"

template<class Serialish>  //Serial_ or HardwareSerial, had to make this a template class due to lack of common base class for different Serials for soem processors.
class EasyConsole: public ChainPrinter {
  public:
    Serialish &conn; //output is wrapped by ChainPrinter

    explicit EasyConsole(Serialish &serial, bool autofeed = false): ChainPrinter(serial,autofeed), conn(serial) {
      //#done
    }

    /** @returns a keystroke, 0 if there are no strokes present. Nulls will get ignored.*/
    byte getKey() {
      if (/*conn &&*/ conn.available()) {//avr usb support has a 10 ms delay each time you check it when it is on. We drop checking for all trusting that available() is fast when port not "on"
        return conn.read();
      } else {
        return 0;
      }
    }

    void begin(uint32_t uartbaud = 115200) {
      conn.begin(uartbaud);//hardware serial. up the baud to reduce overhead.
    }

  //removed as console was often passed as a ChainPrinter and you can't have virtual template members as of C++20 and probably never.
  //template<typename ... Args> EasyConsole& operator()(const Args ... args) {... }

};
