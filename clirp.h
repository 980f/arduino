#pragma once  //(C) 2018-2019 Andy Heilveil, github/980f

#include "cheaptricks.h" //take()
#include "unsignedrecognizer.h"  //recognize numbers but doesn't deal with +/-

/**
  Command Line Interpreter, Reverse Polish input

  If you have a 2-arg function
  then the prior arg is take(pushed)
*/

class CLIRP {
    UnsignedRecognizer numberparser;
    //for 2 parameter commands, gets value from param.
  public://until we get template to work.
    unsigned arg = 0;
    unsigned pushed = 0;
  public:
    /** command processor */
    bool doKey(byte key) {
      if (key == 0) { //ignore nulls, might be used for line pacing.
        return false;
      }
      //test digits before ansi so that we can have a numerical parameter for those.
      if (numberparser(key)) { //part of a number, do no more
        return false;
      }
      arg = numberparser; //read and clear, regardless of whether used.
      switch (key) {//used: aAbcdDefFhHiIjlMmNnoprstwxyzZ  :@ *!,.   tab cr newline
        case '\t'://ignore tabs, makes param files easier to read.
          return false;
        case ','://push a parameter for 2 parameter commands.
          pushed = arg;//by not using take() here 1234,X will behave like 1234,1234X
          return false;
      }
      return true;//we did NOT handle it, you look at it.
    }

    template <typename Ret> Ret call(Ret (*fn)(unsigned, unsigned)) {
      (*fn)(take(pushed), take(arg));
    }

    template <typename Ret> Ret call(Ret (*fn)(unsigned)) {
      pushed = 0; //forget unused arg.
      (*fn)(take(arg));
    }

};
