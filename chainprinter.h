//(C) Copyright 2018, Andrew Heilveil, github/980F.
#pragma once

//F() is misfiring, increasing strings instead of halving them, at least when used with ChainPrinter
#undef F
#define F(arg) arg

#include "cheaptricks.h"  //take()
/** somewhat simple printer extension.
    This wraps a Print object to allow printing a series of fields in one call:
    usage;  ChainPrinter log(somePrinter);
            ...
            log("This", 42, 'c', 12.4, anythingforwhichthereisaprintvariant, HEXLY(0x980F));

    Initial design returns what the Print::print routines do, all nicely added up. This however precludes chaining to an end-of-line function. 
    OTOH since we have variable length args we can add a Newline as an argument.
*/

/** Arduino Print functionality is type aware, and supports an interface class Printable.
    we use that to output numbers in different bases by using this class to pass the number and base as a single argument to the ChainPrinter.
*/
template<typename Intish, int base> struct Basely: public Printable {
  Intish value;
  Basely(Intish value): value(value) {}

  size_t printTo(Print& p) const override {
    return p.print(value, base);
  }
};

/** this class is only useful if:
  1) the choice of base is runtime variable (who does that?)
  or
  2) you need to squeeze program space by sharing code across different bases.
*/
template<typename Intish> struct Based: public Printable {
  Intish value;
  int base;
  Based(Intish value, int base): value(value), base(base) {}

  size_t printTo(Print& p) const override {
    return p.print(value, base);
  }
};


//c++11 version
#define HEXLY(varname) Basely<decltype(varname),16>(varname)

#define BITLY(varname) Basely<decltype(varname),2>(varname)


/** for printing chunks of ram. */
struct BlockDumper : public Printable {
  uint8_t *base;
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


struct ChainPrinter {
    Print &raw;
    bool autofeed;
    explicit ChainPrinter(Print &raw, bool autofeed = false): raw(raw), autofeed(autofeed) {}
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
      return raw.println();
    }

    /** constructing one of these temporarily turns off auto linefeed, destruction restores the autofeed status. Nesting works. */
    class FeedStacker {
        bool wasFeeding;
        bool owner;
      public:
        ChainPrinter &printer;
        explicit FeedStacker(ChainPrinter &printer,bool beFeeding): printer(printer)	{
          wasFeeding = printer.autofeed;
          printer.autofeed= beFeeding;
          owner=true;
        }

        /** make a copy but keep destructor of prior from doing anything, just so parent class can have a factory for these. */
        FeedStacker(FeedStacker &&other): wasFeeding(other.wasFeeding), printer(other.printer) {
          other.owner = false;
        }

        FeedStacker(const FeedStacker &other) = delete;

        ~FeedStacker() {
        	if(owner){
          	printer.autofeed = wasFeeding;
        	}
        }
    };

    /** you must assign this to a named thing to ensure the compiler doesn't elide it.
    suggested usage:  auto pop= printer.stackFeeder(local_preference_for_linefeeding) */
    FeedStacker stackFeeder(bool beFeeding=false) {//default value for legacy upgrade from nofeeds
      return FeedStacker(*this,beFeeding);
    }

};

struct CrLf: public Printable {
  size_t printTo(Print& p) const override {
    return p.println();
  }
};

const CrLf CRLF;
