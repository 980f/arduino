#pragma once

#include "microevent.h"

#include "histogrammer.h" 
/**
  created to see if you are missing milliseconds in your loop().
  call check() often, and then show when it has been a while.

  method operator() combines all the functionality in one call.

*/

#ifndef MicroCheckerSize
#define MicroCheckerSize 100
#endif

//#define DebugMicroChecker 1


#if DebugMicroChecker
#include "dbgserial.h"
#define mydbg(...) dbg(__VAR_ARGS__)
#else
#define mydbg(...)
#endif



template <unsigned qty> class MicroChecker: public Histogrammer<MicroCheckerSize> {
    MicroTick lastCheck = 0;
  public:
    unsigned checks = 0; //for application to decide when to show

    /** call often enough to sample every millisecond value */
    void check(){
      unsigned delta = MilliTicker.since(millilast);
      mydbg("MC:", delta, "/", millilast);
      Histogrammer<qty>(delta);
      millilast = MilliTicker.recent();
      ++checks;
    }

    //inherits unmodified show() and operator()

};
