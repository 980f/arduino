#pragma once //(C)2019 Andy Heilveil, github/980F

#include "chainprinter.h"

template<class Serialish>  //Serial_ or HardwareSerial
class EasyConsole: public ChainPrinter {
  public:
    Serialish &conn; //output is wrapped by ChainPrinter

    EasyConsole(Serialish &serial): ChainPrinter(serial), conn(serial) {
      //#done
    }

    /** @returns a keystroke, 0 if there are no strokes present. Nulls will get ignored.*/
    byte getKey() {
      if (conn && conn.available()) {
        return conn.read();
      } else {
        return 0;
      }
    }

    void begin(uint32_t uartbaud = 115200) {
      conn.begin(uartbaud);//hardware serial. up the baud to reduce overhead.
    }

    template<typename ... Args> EasyConsole& operator()(const Args ... args) {
      if (sizeof... (args)) {
        if (conn) ChainPrinter::operator()(args...);
      }
      return *this;
    }

};
