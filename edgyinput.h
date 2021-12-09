#pragma once
#if debug_edgy
#include "chainprinter.h"
ChainPrinter edbg(Serial, true); //true adds linefeeds to each invocation.
#else
#define edbg(...)
#endif

/** a debouncer that you push samples at, It filters for number of samples n a row that do not match last stable value */
template <typename Scalar>
class EdgyInput {
    Scalar stableValue;
    unsigned inarow;
  public:
    unsigned threshold;

    void configure(unsigned filter) {
      threshold = filter;
    }

    void init(Scalar reading) {
      stableValue = reading;
      inarow = 0;
    }

    operator Scalar() const {
      return stableValue;
    }

    /** @returns whether it just became stable */
    bool operator ()(Scalar reading) {
      if (stableValue == reading) {
        inarow = 0;
        return false;
      }
      //else it is different
      if (++inarow >= threshold) {
        init(reading);
        return true;
      }
      return false;
    }

    /** @returns that the related input is still the last value that it was stable at, that it is not transitioning */
    bool isSteady()const {
      return inarow == 0;
    }

    /** expose inner count for debug */
    unsigned bouncing() const {
      return inarow;
    }

};
