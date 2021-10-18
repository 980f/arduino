
#include "digitalpin.h"
#include "cheaptricks.h" //::changed


#ifdef debug_edgy
#include "chainprinter.h"
ChainPrinter edbg(Serial, true); //true adds linefeeds to each invocation.
#else
#define edbg(...)
#endif


class EdgyInput {
    bool last; //last stable value
    const DigitalInput &pin;
    const unsigned filter;
  public: //for debug
    unsigned debouncer;
    unsigned changes = 0;
  public:
    EdgyInput(const DigitalInput &which, unsigned filter = 0): pin(which), filter(filter) {
      //do nothing until setup
    }

    void begin() {
      changes = 0;
      debouncer = 0;
      last = raw();
    }

    /** @returns last stable state */
    operator bool () {
      return last;
    }

    /** @returns the pin. This is a step towards extracting a base class not tied to digital pins */
    bool raw() {
      return bool(pin);
    }

    /** @returns whether it has become stable in a new state since the last time this was called */
    bool changed() {
      if (filter > 0) {
        return take(changes) > 0;
      } else {
        return ::changed(last, raw());
      }
    }

    /** must be called periodically, @returns whether it has changed. */
    bool onTick() {
      if (last != raw()) {
        edbg("changing from:", last);
        if (debouncer++ >= filter) {
          last = !last;//it has changed
          ++changes;
          edbg( "changed to:", last);
          return true;
        } else {
          edbg(debouncer, "<", filter);
        }
      } else {
        if (debouncer) {
          edbg("glitched ", !last, " for ", debouncer);
        }
        debouncer = 0;
        //        edbg("Stable: ",last);
      }
      return false;
    }

};
