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
    unsigned millihist[numhist + 1];//1-based access for codign convenience, 0th entry unused
    MilliTick millilast = 0;
  public:
    unsigned checks = 0; //for application to decide when to show

    /** call often enough to sample every millisecond value */
    void check();
    /** shows the histogram on Print @param dbg, and if @param andClear then zero the 'gram.
    */
    bool show(Print &dbg, bool andClear = true) ;

    //update and if it has been @param modulus checks show the histogram and if @param andClear then zero the 'gram.
    bool operator ()(Print &dbg, unsigned modulus, bool andClear = true);

};
