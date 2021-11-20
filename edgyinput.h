
#include "digitalpin.h"
#include "cheaptricks.h" //::changed


#ifdef debug_edgy
#include "chainprinter.h"
ChainPrinter edbg(Serial, true); //true adds linefeeds to each invocation.
#else
#define edbg(...)
#endif



// will extract to own file soon:
/** a debouncer that you push samples at, It filters for number of samples */
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

    bool isSteady()const {
      return inarow == 0;
    }

};
