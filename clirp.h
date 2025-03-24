#pragma once  //(C) 2018,2019,2021 Andy Heilveil, github/980f

#ifndef Arduino
using byte = unsigned char;
#endif

#include "unsignedrecognizer.h"  //recognize numbers but doesn't deal with +/-
#include "cheaptricks.h"  //ClearOnRead<> COR.

/**
  Command Line Interpreter, Reverse Polish input

  You push bytes at it, then query it for values

  if(clirp.doKey(key_from_serial)){
    switch(key){
    case someletter:
      auto result=cli(pointer_to_myfunction); //calls function of one or two arguments. also zeroes out the stored arguments.
      //auto is the return type of 'myfunction'
    break;
    }
  }

  todo: upgrade to floating point recognizer via 'template if' on class to use.
  useNaV: template arg to use ~0 instead of 0 for 'empty' numbers (NaV is Not a Value, similar to NaN not a number)
  todo: template arg for max number of arguments and use an array rather than two named variables. Needs fancy template varargs stuff presently beyond the author's abilities, also need to check AVR compiler versions for feature support
*/
template<typename Unsigned = unsigned, bool useNaV = false>
class CLIRP {
    UnsignedRecognizer<Unsigned> numberparser;
  public:
    enum {
      Empty = useNaV ? ~0 : 0
    };
    //COR is "Clear on Read", accessing the value naively returns present value but sets storage to Empty.
    COR<Unsigned> arg = Empty;
    //for 2 parameter commands, pushed gets value from earlier param.
    COR<Unsigned> pushed = Empty;
  public:
    /** command processor. pass it each char as they arrive.
      @returns false if char was used internally, true if you should inspect it*/
    bool doKey(byte key) {
      //Serial.println("Key:%d",key);
      if (key == 0) { //ignore nulls, might be used for line pacing.
        return false;
      }
      if (key == 3) { //^C
        pushed = Empty;
        arg = Empty;
        numberparser.clear();
      }
      if (key == 255) { //ignore failure of caller to check for ~0 return when reading and nothing present.
        return false;
      }
      //test digits before ansi so that we can have a numerical parameter for those.
      if (numberparser(key)) { //part of a number, do no more
        return false;
      }
      //ansi escape sequence doober would go here if we bring it back. it is a state machine that accumulates ansi sequence and stored it on a member herein, returning true when sequence complete.
      arg = numberparser; //read and clear, regardless of whether used.
      switch (key) {
        case '\t'://ignore tabs, makes param files easier to read.
          return false;
        case ','://push a parameter for 2 parameter commands.
          pushed = arg;//note: pushing clears accumulator  123,? acts the same as 123,0?  (early versions gave you 123,123? which was never useful)
          return false;
      }
      return true;//we did NOT handle it, YOU should look at it.
    }

    /**  may deprecate doKey if this doesn't interfere with other operator() usages. */
    bool operator()(byte key) {
      return doKey(key);
    }

    /** @returns whether there is a second non-zero argument. Use 1-based labeling and have labels precede values when doing array assignments. */
    bool twoargs() const {
      return bool(pushed);
    }


    /** @returns 2 if pushed arg is not Empty, ELSE 1 if single arg is not Empty, ELSE 0.
      caveat: Early versions wrongly ignored useNAV in the tests for empty values.
    */
    unsigned argc() const {
      return (pushed != Empty) ? 2 : (arg != Empty) ? 1 : 0;//if pushed reply 2 regardless of whether arg appears to have a value.
    }

    /** @returns parameter, but also clears it for next command, you can only read once per command */
    Unsigned operator[](unsigned index) {
      return index ? pushed : arg;
    }

    /** Call the @param fn with the arguments present and @returns what that function returned.
      NB: the argument order is the reverse of the RPN entry order. We can debate whether this is a good choice, but it matches early usage of this class.
      E.G.: if an array is being accessed as index,valueX for array[index]=value, the fn is passed (value,index)
    */
    template <typename Ret, typename U1, typename U2> Ret operator()(Ret (*fn)(U1, U2)) {
      return (*fn)(arg, pushed);
    }

    /** Call the @param fn with the most recent argument, erasing any prior one and @returns what that function returned.
    */
    template <typename Ret, typename U1> Ret operator()(Ret (*fn)(U1)) {
      pushed = Empty; //forget unused arg.
      return (*fn)(arg);
    }

};
