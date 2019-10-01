//(C) Copyright 2018, Andrew Heilveil, github/980F.
#pragma once

/** simplest printer extension.
 *  This wraps a Print object to allow printing a series of fields in one call:
 *  usage;  ChainPrinter chp(somePrinter); 
 *          ...
 *          chp("This", 42, 'c', 12.4, anythingforwhichthereisaprintvariant);
 *          
 *  Initial design returns what the Print::print routines do, all nicely added up. This however precludes chaining to an end-of-line function.
*/

struct ChainPrinter {
    Print &raw;
    ChainPrinter(Print &raw): raw(raw) {}
  private:
    /** this is how you process the nth item of a varargs template group.
        It can generate a surprising amount of code, a function for every combination of argument types, AND all right hand subsets thereof.
        Fortunately multiple print statements with the same argument types share code, there is no dependency on the format arg or the argument values. */


    template<typename First, typename ... Args> unsigned PrintItem(First &&first, Args&& ... args) {
      return raw.print(first) + PrintItem(args ...);
    }

    template<typename Single> unsigned PrintItem(Single &&arg) {
      return raw.print(arg);
    }

  public:
    template<typename ... Args> unsigned operator()(const Args ... args) {
      if (sizeof... (args)) {
        return PrintItem(args ...);
      } else {
        return 0;
      }
    }

    //print with a newline after all the given args, 
    template<typename ... Args> unsigned line(const Args ... args) {
      return operator()(args ...)+endl();
    }

    unsigned endl() {
      return raw.println();
    }

};
