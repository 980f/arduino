#pragma once  //(C) 2018-2019 Andy Heilveil, github/980f

#include "cheaptricks.h" //take()
#include "unsignedrecognizer.h"  //recognize numbers but doesn't deal with +/-

/**
  Command Line Interpreter, Reverse Polish input

  If you have a 2-arg function
  then the prior arg is pushed
*/

template<typename Unsigned>
class CLIRP {

    UnsignedRecognizer<Unsigned> numberparser;
    //for 2 parameter commands, pushed gets value from param.
  public:
    COR<Unsigned> arg = 0;
    COR<Unsigned> pushed = 0;
  public:
    using Value = Unsigned;
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
          pushed = arg;//pushing now clears accumulator  123,X => 123,0X
          return false;
      }
      return true;//we did NOT handle it, you look at it.
    }

    bool twoargs() const {
      return bool(pushed);
    }

		/** NB: the argument order is the reverse of the RPN entry order. We can debate whether this is a good choice, but it matches early usage of this class.*/
    template <typename Ret, typename U1, typename U2> Ret operator()(Ret (*fn)(U1, U2)) {
      return (*fn)(arg,pushed);
    }

    template <typename Ret, typename U1> Ret operator()(Ret (*fn)(U1)) {
      pushed = 0; //forget unused arg.
      return (*fn)(arg);
    }

};
