#pragma once //(C) 2018,2019,2021 Andy Heilveil, github/980f

#ifndef Arduino
using byte = unsigned char;
#endif

#include "unsignedrecognizer.h" //recognize numbers but doesn't deal with +/-

/**
  Command Line Interpreter, Reverse Polish input

  You push bytes at it, then query it for values

  if(clirp(key_from_serial)){
    switch(key_from_serial){
    case someletter:
      auto result=cli(pointer_to_myfunction); //calls function of one or two arguments. also zeroes out the stored arguments.
      //auto is the return type of 'myfunction'
    break;
    }
  }

  todo: upgrade to floating point recognizer via 'template if' on class to use.
  todo: replace useNaV with template arg to use ~0 instead of 0 for 'empty' numbers (NaV is Not a Value, similar to NaN not a number)
  tested: template arg for max number of arguments and use an array rather than two named variables. Need to check AVR compiler versions for feature support and give clear error message vs weird spew.
*/
template<typename Unsigned = unsigned, bool useNaV = false, unsigned maxArgs = 2>
class CLIRP {
  UnsignedRecognizer<Unsigned> numberparser;
  
public:
  enum {
    Empty = useNaV ? ~0 : 0
  };
  Unsigned argv[maxArgs];
  unsigned argc = 0;

  void reset() {
    for (argc = maxArgs; argc > 0;) {
      argv[--argc] = Empty;
    }
    //formerly argc would be left at MAXUNSIGNED rather than the 0 the comment here suggested, and which then wiped all of ram the next time an arg stack push occured.
  }

  void pushArgs(Unsigned newarg){
    if(argc > 0){
      for(unsigned i = min(argc, maxArgs-1); i > 0; --i){//the min() deals with overflowing the 'stack'
        argv[i] = argv[i-1];//can't --i here because that is officially undefined behavior, no promise that the array element addresses won't both be computed before the assignement is executed.
      }
    }
    argv[0] = newarg;
    if (argc < maxArgs) {
      ++argc;
    }    
  }

public:
  /** command processor. pass it each char as they arrive.
    @returns false if char was used internally, true if you should inspect it */
  bool doKey(byte key) {
    if (key == 0) { // ignore nulls, might be used for line pacing.
      return false;
    }

    if (key == 255) { // ignore failure of caller to check for ~0/-1 return when reading and nothing present.
      return false;
    }

    if (key == 3) { //^C
      reset();
      numberparser.clear();
      return false; // this was missing for many versions, not sure how that would have been buggy.
    }

    if (numberparser(key)) { // part of a number, do no more
      return false;
    }
    switch (key) {
      case '\t': // tab same as comma
      case ',': // push a parameter for multi-parameter commands.
        pushArgs(numberparser.notEmpty() ? numberparser: Empty ); //legacy from first implementation, use local 'Empty' rather than unsignedrecognizer's idea of empty.
        return false;
      default:
        if(numberparser.notEmpty()){//if a number was in progress capture it, else leave argstack untouched
          pushArgs(numberparser);
        }
        break;
    }
    return true; // we did NOT handle it, YOU should look at it.
  }

  /**  may deprecate doKey if this doesn't interfere with other operator() usages. */
  bool operator()(byte key) {
    return doKey(key);
  }

  /** @returns whether user entered at least @param argnumber -1 values (0 for any, 1 for 2, sorry but that is the way life goes with 'C' */
  bool has(unsigned argnumber) {
    return argnumber < argc;
  }

  /** @returns parameter */
  Unsigned operator[](unsigned index) const {
    return index < argc ? argv[index] : Empty;
  }

  /** Call the @param fn with the arguments present and @returns what that function returned.
    NB: does NOT remove the arguments from the argument stack, the caller of this function must arrange for that, usually by returning true from the keyhandler.
    NB: the argument order is the reverse of the RPN entry order. We can debate whether this is a good choice, but it matches early usage of this class.
    E.G.: if an array is being accessed as index,valueX for array[index]=value, the fn is passed (value,index)
  */
  template<typename Ret, typename U1, typename U2> Ret operator()(Ret (*fn)(U1, U2)) {
    return (*fn)(argv[0], argv[1]);
  }

  /** Call the @param fn with the most recent argument @returns what that function returned.
    NB: does NOT remove the arguments from the argument stack, the caller of this function must arrange for that, usually by returning true from the keyhandler.
   */
  template<typename Ret, typename U1> Ret operator()(Ret (*fn)(U1)) {
    return (*fn)(argv[0]);
  }
};
