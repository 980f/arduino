//(C) Copyright 2018, Andrew Heilveil, github/980F.
#pragma once

//avr is missing this stuff: #include <type_traits>
/** somewhat simple printer extension.
    This wraps a Print object to allow printing a series of fields in one call:
    usage;  ChainPrinter log(somePrinter);
            ...
            log("This", 42, 'c', 12.4, anythingforwhichthereisaprintvariant, HEXLY(0x980F));

    Initial design returns what the Print::print routines do, all nicely added up. This however precludes chaining to an end-of-line function. OTOH since we have variable length args we can add a Newline as an argument.
*/

/** Arduino Print functionality is type aware, and supports an interface class Printable.
 *  we use that to output numbers in different bases by using this class to pass the number and pass as a single argument to the ChainPrinter.
*/
template<typename Intish,int base>
struct Basely: public Printable {
  Intish value;
  Basely(Intish value): value(value) {}

  size_t printTo(Print& p) const {
    return p.print(value,base);
  }
};

/** this class is only useful if:
1) the choice of base is runtime variable (who does that?)
or
2) you need to squeeze program space by sharing code across different bases. 
*/
template<typename Intish>
struct Based: public Printable {
  Intish value;
  int base;
  Based(Intish value,int base): value(value),base(base) {}

  size_t printTo(Print& p) const {
    return p.print(value,base);
  }
};


//c++11 version 
#define HEXLY(varname) Basely<decltype(varname),16>(varname)

#define BITLY(varname) Basely<decltype(varname),2>(varname)


struct ChainPrinter {
    Print &raw;
    ChainPrinter(Print &raw): raw(raw) {}
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
    template<typename ... Args> unsigned operator()(const Args ... args) {
      if (sizeof... (args)) {//this check keeps us from having to implement a no-args PrintItem. 
        return PrintItem(args ...);
      } else {
        return 0;
      }
    }

    //print with a newline after all the given args,
    template<typename ... Args> unsigned line(const Args ... args) {
      return operator()(args ...) + endl();
    }

    unsigned endl() {
      return raw.println();
    }

};
