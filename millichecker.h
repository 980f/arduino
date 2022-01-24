#pragma once

#include "millievent.h"

#define DebugMillichecker 0


#if DebugMillichecker

#include "chainprinter.h"
extern ChainPrinter dbg;

#else
#define dbg(...) 
#endif

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
    unsigned millihist[numhist + 1];//1-based access for codign convenience, 0th entry unused
    MilliTick millilast = 0;
  public:
    unsigned checks = 0; //for application to decide when to show

    /** call often enough to sample every millisecond value */
    void check() {
      unsigned delta = MilliTicker.since(millilast);
      
      dbg("MC:",delta,"/",millilast);
      
      ++ millihist[min(delta, numhist)];
      millilast = MilliTicker.recent();
      ++checks;
    }

    /** shows the histogram on Print @param dbg, and if @param andClear then zero the 'gram.
    */
    bool show(Print &dbg, bool andClear = true) {
      unsigned nonzeroes = 0;
      for (unsigned hi = numhist; hi > 0; --hi) { //ignore 0th entry
        if (millihist[hi] > 0) {//zeroes are usually very common at the high end of the range so we drop all as a convenient way of dropping those.
          //if 1 is the first non-zero entry then print nothing
          if (hi > 1 || nonzeroes > 0) {
            if(nonzeroes==0){
              dbg.println();//ensure alignment of output, but only if we are definitely outputing something.
            }
            dbg.print(hi); dbg.print(":\t"); dbg.println(millihist[hi]);
          }
     
          ++nonzeroes;
          if (andClear) {
            millihist[hi] = 0;
          }
        }
      }
      return nonzeroes!=0;
    }

    //update and if it has been @param modulus checks show the histogram and if @param andClear then zero the 'gram.
    bool operator ()(Print &dbg, unsigned modulus, bool andClear = true) {
      check();
      if (checks % modulus == 0) {
        return show(dbg, andClear);
      }
      return false;
    }

};
