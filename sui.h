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

  using User = void(*)(unsigned char /*key*/, bool /*upper*/, decltype(cli) &/*argset*/);

  void operator()(User handler) {
    for (unsigned strokes = cin.available(); strokes-- > 0;) {
      processKey(cin.read(), handler);
    }
  }

  //extracted to allow multiple input streams, which streams will not mix nicely
  void processKey(int key, User handler) {
    if (cli(key)) {
      bool upper = key < 'a';
      handler(tolower(key), upper, cli);
    }
  }

  unsigned operator[](unsigned pi) {
    return cli[pi];
  }

  unsigned numParams() const {
    return cli.argc();
  }

};
