
#include "digitalpin.h"
#include "cheaptricks.h" //::changed

#include "chainprinter.h"
ChainPrinter edbg(Serial, true); //true adds linefeeds to each invocation.



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

    operator bool () {
      if (last != raw()) {
        edbg("changing from:", last);
        if (debouncer++ >= filter) {
          last = !last;//it has changed
          ++changes;
          edbg( "changed to:", last);
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
      return last;
    }

    bool raw() {
      return bool(pin);
    }

    bool changed() {
      if (filter > 0) {
        return take(changes) > 0;
      } else {
        return ::changed(last, raw());
      }
    }

};
