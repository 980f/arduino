#pragma once

#include "millievent.h"

#include "histogrammer.h" 
/**
  created to see if you are missing milliseconds in your loop().
  call check() often, and then show when it has been a while.

  method operator() combines all the functionality in one call.

*/

#ifndef MilliCheckerSize
#define MilliCheckerSize 100
#endif

//#define DebugMillichecker 1


#if DebugMillichecker
#include "dbgserial.h"
#define mydbg(...) dbg(__VAR_ARGS__)
#else
#define mydbg(...)
#endif



template <unsigned qty> 
class MilliChecker: public Histogrammer<qty> {
    MilliTick millilast = 0;
  public:
    unsigned checks = 0; //for application to decide when to show

    /** call often enough to sample every millisecond value */
    void check();

  unsigned delta = MilliTicker.since(millilast);

  mydbg("MC:", delta, "/", millilast);
  Histogrammer<qty>(delta);
  millilast = MilliTicker.recent();
  ++checks;
}

    
    //inherits unmodified show() and operator()

};
