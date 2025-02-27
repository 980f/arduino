#pragma once

#include "millievent.h"

#include "histogrammer.h"
/**
  created to see if you are missing milliseconds in your loop().
  call check() often, and then show when it has been a while.

  method operator() combines all the functionality in one call.

*/

//#define DebugMillichecker 1

#if DebugMillichecker
#include "dbgserial.h"
#define mydbg(...) dbg(__VAR_ARGS__)
#else
#define mydbg(...)
#endif



template <unsigned qty> class MilliChecker: public Histogrammer<qty> {
    MilliTick millilast = 0;
  public:
    /** call often enough to sample every millisecond value */
    void check() {
      unsigned delta = MilliTicker.since(millilast);

      mydbg("MC:", delta, "/", millilast);
      Histogrammer<qty>::check(delta);
      millilast = MilliTicker.recent();
    }

    //inherits unmodified show() and operator()

};
