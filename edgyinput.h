
#include "digitalpin.h"
#include "cheaptricks.h" //::changed

class EdgyInput {
    bool last; //last stable value
    const DigitalInput pin;
    unsigned debouncer;
    unsigned filter;
  public:
    EdgyInput(unsigned which, unsigned filter = 0): pin(which), filter(filter) {
      last = this->raw();//elaborate in order to check compiler
    }


    operator bool () {
      if (last != raw()) {
        if (debouncer++ >= filter) {
          last = !last;//it has changed
        }
      } else {
        debouncer = 0;
      }
      return last;
    }

    bool raw() {
      return bool(pin);
    }

    bool changed() {
      if (filter > 0) {
        if (debouncer >= filter) {
          debouncer = 0;
          return true;
        }
        return false;
      } else {
        return ::changed(last, raw());
      }

    }

};
