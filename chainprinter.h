//(C) Copyright 2018,2022 Andrew Heilveil, github/980F.
#pragma once
#include <Arduino.h>
//F() is misfiring, increasing strings instead of halving them, at least when used with ChainPrinter
#undef F
#define F(arg) arg

/** somewhat simple printer extension.
    This wraps a Print object to allow printing a series of fields in one call:
    usage:  ChainPrinter log(somePrinter);
            ...
            log("This", 42, 'c', 12.4, anythingforwhichthereisaprintvariant, HEXLY(0x980F));

    Initial design returns what the Print::print routines do, all nicely added up. This however precludes chaining to an end-of-line function.
    OTOH since we have variable length args we can add a Newline as an argument. (Later on added auto newline state, using empty param list as trigger).

   Arduino Print functionality is type aware, and supports an interface class Printable.
    we use that to output numbers in different bases by using this class to pass the number and base as a single argument to the ChainPrinter.

  ChainPrinter has state to enable generating an end of line after the last argument.
  A FeedStacker is an RAII object that lets you temporarily change the automatic linefeed state for a block of your code.

  There is a shared object named CRLF that you may put in your argument list instead of '\n' or "\r\n". It emits whatever Arduino is configured to emit for a line ending.

  The 'stifled' member of ChainPrinter was tacked on to suppress attempt to talk to SerialUSB before it is ready, without making an app wait for it.
  If you wait for it you may wait forever, most usages end up requiring a live PC connection to allow the Arduino program to loop.
  You may choose to use it as an enable for a debug stream, where enabling it use stifled=!Serial rather than just 'false'.

*/

/** binds a base and a value to print in that base */
template<typename Intish, int base> struct Basely: public Printable {
  Intish value;
  Basely(Intish value): value(value) {}

  size_t printTo(Print& p) const override {
    return p.print(value, base);
  }
};

/** Based class is only useful if:
  1) the choice of base is runtime variable (who does that?)
  or
  2) you need to squeeze program space by sharing code across different bases.
  
  @see Basely for when base is known at compile time
*/
template<typename Intish> struct Based: public Printable {
  const Intish value;
  int base;
  Based(Intish value, int base): value(value), base(base) {}

  size_t printTo(Print& p) const override {
    return p.print(value, base);
  }
};


//c++11 version
#define HEXLY(varname) Basely<decltype(varname),16>(varname)

#define BITLY(varname) Basely<decltype(varname),2>(varname)


/** for printing chunks of ram.
  While you can invoke it via passing a Print to its printTo method, you can also pass the object to a ChainPrinter to get some framing around it in one line of code. */
struct BlockDumper : public Printable {
  const uint8_t *base;
  unsigned length;
  char comma;
  BlockDumper(uint8_t *base, unsigned length, char comma = ' '): base(base), length(length), comma(comma)  {  }

  size_t printTo(Print& p) const override {
    size_t emitted = 0;
    for (size_t i = 0; i < length; ++i) {
      emitted += p.print(base[i], HEX);
      if (comma) {
        emitted += p.print(comma);
      }
    }
    return emitted;
  }
};

#define BLOCK(...) BlockDumper( __VA_ARGS__ )


#include "valuestacker.h" //a stack that pushes on object declaration and pops on object destruction.

using FlagStacker=ValueStacker<bool>;


class ChainPrinter {
  public://this is Arduino land,who needs protection!?
    Print &raw;
  public://simple user set state
    bool autofeed; //whether to emit CRLF when end of arglist is encountered, which is the same as passing noargs. myChainPrinter() emits a CRLF.
    bool stifled = true; //startup disabled due to SerialUSB issues.
    explicit ChainPrinter(Print &raw, bool autofeed = true): raw(raw), autofeed(autofeed) {}
    //hope this makes 'write()' methods available 
    
  private:
    /** this is how you process the nth item of a varargs template group.
        It can generate a surprising amount of code, a function for every combination of argument types, AND all right hand subsets thereof.
        Fortunately multiple print statements with the same argument types share code, there is no dependency on the argument values. */
    template<typename First, typename ... Args> unsigned PrintItem(First &&first, Args&& ... args) {
      return raw.print(first) + PrintItem(args ...);
    }

    /** this is how you terminate processing a varargs template */
    template<typename Single> unsigned PrintItem(Single &&arg) {
      return raw.print(arg);
    }

  public:
    /** by overloading operator () we can make invocations look like common logging functions calls. Name your chainprinter dbg or log.*/
    template<typename ... Args>  unsigned operator()(const Args ... args) {
      if (stifled) return 0;
      if (sizeof... (args)) {//this check keeps us from having to implement a no-args PrintItem.
        return PrintItem(args ...) + (autofeed ? endl() : 0);
      } else {
        return 0;
      }
    }

    //print with a newline after all the given args, use when autofeed is off to mark last entities.
    template<typename ... Args> unsigned line(const Args ... args) {
      return operator()(args ...) + endl();
    }

    unsigned endl() {    
      if (stifled) return 0;
      return raw.println();
    }

    /** you must assign this to a named thing to ensure the compiler doesn't elide it.
      suggested usage:  auto pop= printer.stackFeeder(local_preference_for_linefeeding) 
      default value for arg disables feeding while the returned object exists */
    FlagStacker stackFeeder(bool beFeeding = false) {
      return FlagStacker(this->autofeed, beFeeding);
    }

    /** you must assign this to a named thing to ensure the compiler doesn't elide it.
          suggested usage:  auto pop= printer.stackStifled(local_preference_for_debugspew)          
          */
    FlagStacker stackStifled() {//earlier version allowed you to force a stifled printer back on, a universally bad idea.
      return FlagStacker(this->stifled, true);
    }
};

struct CrLf: public Printable {
  size_t printTo(Print& p) const override {
    return p.println();
  }
};

/** we can share this */
const CrLf CRLF;
