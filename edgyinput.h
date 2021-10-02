
#include "digitalpin.h"
#include "cheaptricks.h" //::changed

class EdgyInput {
    bool last;
    const DigitalInput pin;
  public:
    EdgyInput(unsigned which): pin(which) {
      last = this->operator bool();
    }

    operator bool () {
      return bool(pin);
    }

    bool changed() {
      return ::changed(last, *this);
    }

};
