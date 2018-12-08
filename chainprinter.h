#pragma once

/** simplest printer extension */
struct ChainPrinter {
    Print &raw;
    ChainPrinter(Print &raw): raw(raw) {}
  private:
    /** this is how you process the nth item of a varargs template group.
        It can generate a surprising amount of code, a function for every combination of argument types, AND all right hand subsets thereof.
        Fortunately multiple print statements with the same argument types share code, there is no dependency on the format arg or the argument values. */
 

    template<typename First, typename ... Args> unsigned PrintItem(First &&first, Args&& ... args) {
      if (sizeof... (args)) {
        return raw.print(first) + PrintItem(args ...);
      } else {
        return raw.print(first);
      }
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

};
