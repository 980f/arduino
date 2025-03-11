#pragma once

#include "chainprinter.h"

#include "clirp.h" //command-line interpreter with reverse polish input, all args precede operator.

/**
  2nd version, prepare to make #args expandable while fixing other utility issues
*/

struct SUI { //Simple User Interface. Binds together a console and an RPN command parser.
  CLIRP<> cli;
  decltype(Serial) &cin;//some platforms have different declared classes for symbol Serial.
  ChainPrinter cout;

  SUI (decltype(Serial) &keyboard, Print&printer): cin(keyboard), cout(printer, true) {}

  using User = void(*)(unsigned char /*key*/, bool /*upper*/);

  void operator()(User handler) {
    for (unsigned strokes = cin.available(); strokes-- > 0;) {
      auto key = cin.read();
      if (cli.doKey(key)) {
        bool upper = key < 'a';
        handler(tolower(key), upper);
      }
    }
  }

  unsigned operator[](unsigned pi) {
    return pi > 0 ? cli.pushed : cli.arg;
  }

  unsigned numParams() const {
    return cli.twoargs() ? 2 : bool(cli.arg);
  }

};
