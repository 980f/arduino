#pragma once

#include "millievent.h"

/**
  utility to see if you are missing milliseconds in your loop().
  call check() often, and then show when it has been a while.

  method operator() combines all the functionality in one call. 

*/

#ifndef MilliCheckerSize
#define MilliCheckerSize 100
#endif

class MilliChecker {
    static const unsigned numhist = MilliCheckerSize;
    unsigned millihist[numhist + 1];
    unsigned millilast = 0;
  public:
    unsigned checks = 0; //for application to decide when to show

    /** call often enough to sample every millisecond value */
    void check() {
      unsigned delta = MilliTicked.since(millilast);
      ++ millihist[min(delta, numhist)];
      millilast = MilliTicked.recent();
      ++checks;
    }

    /** shows the histogram on Print @param dbg, and if @param andClear then zero the 'gram.
    */
    void show(Print &dbg, bool andClear = true) {
      dbg.println();//ensure alignment of output
      for (unsigned hi = numhist + 1; hi-- > 0;) {
        if (millihist[hi] > 0) {//zeroes are usually very common at the high end of the range so we drop all as a convenient way of dropping those.
          dbg.print(hi); dbg.print(":\t"); dbg.println(millihist[hi]);
          if (andClear) {
            millihist[hi] = 0;
          }
        }
      }
    }

    //update and if it has been @param modulus checks show the histogram and if @param andClear then zero the 'gram.
    bool operator ()(Print &dbg, unsigned modulus, bool andClear = true) {
      check();
      if (checks % modulus == 0) {
        show(dbg, andClear);
        return true;
      }
      return false;
    }

};
