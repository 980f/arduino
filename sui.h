#pragma once

#include "chainprinter.h"

#include "clirp.h"  //command-line interpreter with reverse polish input, all args precede operator.

/**
  2nd version, prepare to make #args expandable while fixing other utility issues
  also give up on composition, derivation is actually a lesser burden all around.
*/
template<typename Unsigned = unsigned, bool useNaV = false, unsigned maxArgs = 2>
struct SUI {  //Simple User Interface. Binds together a console and an RPN command parser.
  CLIRP<Unsigned, useNaV, maxArgs> cli;
  Stream &cin;  //some platforms have different declared classes for symbol Serial.
  ChainPrinter cout;

  SUI(Stream &keyboard, Print &printer): cin(keyboard), cout(printer, true) {}

  //call this in your loop handler
  void loop() {
    for (unsigned strokes = cin.available(); strokes-- > 0;) {
      processKey(cin.read());
    }
  }

  //extracted to allow multiple input streams, which streams will not mix nicely
  void processKey(int key) {
    if (cli(key)) {
      bool upper = key < 'a';
      handleKey(tolower(key), upper);
      cli.reset();
    }
  }

  unsigned operator[](unsigned pi) {
    return cli[pi];
  }

  unsigned numParams() const {
    return cli.argc();
  }

  virtual void handleKey(unsigned char,bool)=0;

};
