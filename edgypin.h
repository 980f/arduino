#pragma once

#include "edgyinput.h"  //generic input debouncer
#include  "digitalpin.h"

/** debounced input pin. */
class EdgyPin : public EdgyInput<bool> {
    const DigitalInput pin;
  public:
    EdgyPin (unsigned arduinoNumber, DigitalPin::Datum polarity, unsigned filter): pin(arduinoNumber, polarity) {
      configure(filter);
    }

    //FYI: operator bool in base class returns last steady state 
    
    /** @returns whether this just changed */
    bool onTick() {
      return (*this)(pin);
    }

    /** call from setup() */
    void begin() {
      //DigitalPin inits itself, no action needed here.
      init(pin);
    }

    bool raw() const {
      return pin;
    }
};
