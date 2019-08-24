#pragma once  //(C) 2018-2019 Andy Heilveil, github/980f

#include "cheaptricks.h" //take()
#include "unsignedrecognizer.h"  //recognize numbers but doesn't deal with +/-

/**
  Command Line Interpreter, Reverse Polish input

  If you have a 2-arg function
  then the prior arg is take(pushed)
*/

template<typename Unsigned>
class CLIRP {
    UnsignedRecognizer<Unsigned> numberparser;
    //for 2 parameter commands, gets value from param.
  public://until we get template to work.
    Unsigned arg = 0;
    Unsigned pushed = 0;
  public:
    /** command processor. pass it each char as they arrive.
    @returns false if char was used internally, true if you should inspect it*/
    bool doKey(byte key) {
      if (key == 0) { //ignore nulls, might be used for line pacing.
        return false;
      }
      //test digits before ansi so that we can have a numerical parameter for those.
      if (numberparser(key)) { //part of a number, do no more
        return false;
      }
      //ansi escape sequence doober would go here if we bring it back. it is a state machine that accumulats ansi sequence and stored it on a member herein, returning true when sequence complete.
      arg = numberparser; //read and clear, regardless of whether used.
      switch (key) {
        case '\t'://ignore tabs, makes param files easier to read.
          return false;
        case ','://push a parameter for 2 parameter commands.
          pushed = arg;//by not using take() here 1234,X will behave like 1234,1234X
          return false;
      }
      return true;//we did NOT handle it, you look at it.
    }

    template <typename Ret,typename U1,typename U2> Ret call(Ret (*fn)(U1, U2)) {
      return (*fn)(take(pushed), take(arg));
    }

    template <typename Ret,typename U1> Ret call(Ret (*fn)(U1)) {
      pushed = 0; //forget unused arg.
      return (*fn)(take(arg));
    }

};
