#pragma once

#include "chainprinter.h"

#include "clirp.h"  //command-line interpreter with reverse polish input, all args precede operator.

/**
  2nd version, prepare to make #args expandable while fixing other utility issues
  also give up on composition, derivation is actually a lesser burden all around.

usage:
  struct MySui: public SUI {
    //your local state
     bool handleKey(unsigned char cmd ,bool wasUpper){
      switch(cmd){
        case 'x': 
          //process x command
          return true;
        case '\e':
          //set some state that says we have recognized part of a multi-letter command
          return false;
      }
      //unrecognized command stuff gets to here:
      //perhaps inform user that they have input garbage.
      return true;//discard
     }
  }


*/
template<typename Unsigned = unsigned, bool useNaV = false, unsigned maxArgs = 2>
struct SUI {  //Simple User Interface. Binds together a console and an RPN command parser.
  CLIRP<Unsigned, useNaV, maxArgs> cli;
  Stream &cin;  //some platforms have different declared classes for symbol Serial.
  ChainPrinter cout; //while not essential inside SUI, most 'handleKey' implementations were printing stuff making it very convenient to include this in the class.

  SUI(Stream &keyboard, Print &printer): cin(keyboard), cout(printer, true) {}

  //call this in your loop handler
  void loop() {
    for (unsigned strokes = cin.available(); strokes-- > 0;) {//only read available() once, to avert consuming the system if serial data is swamping us.
      processKey(cin.read());
    }
  }

  //extracted/exposed to allow multiple input streams, which streams will not mix nicely
  void processKey(int key) {
    if (cli(key)) { //then key was not absorbed by message tokenizer, so it is a command token.
      bool upper = key < 'a';
      if(handleKey(tolower(key), upper)){
        cli.reset();
      }      
    }
  }

  unsigned operator[](unsigned pi) {
    return cli[pi];
  }

  unsigned numParams() const {
    return cli.argc();
  }

  /** @return whether the command arguments have been consumed. */
  virtual bool handleKey(unsigned char,bool)=0;

};
