
#pragma once

/** handler for a clock/dir type device. Works with many quadrature encoders, but geared towards those that are only stable when both signals are high. 
 * see the MakeKnob macro for the only valid means of instantiating onr of these.*/

template<unsigned clockPN, unsigned directionPN, void (*handler)(), typename Knobish = uint8_t> class RotaryKnob {
    Knobish location;

    const InputPin<clockPN, LOW> clock;
    const InputPin<directionPN, LOW> direction;

  public:
    RotaryKnob () {
      zero();
      //Arduino doesn't (yet;) support this:      auto handleKnob= [&](){update()};
      attachInterrupt(digitalPinToInterrupt(clockPN), handler, FALLING);
    }

    void zero() {
      location = 0;
    }

    void update() {
      location += direction ? -1 : 1;
    }
    /** getter for location */
    operator Knobish()const {
      return location;
    }

};


#define MakeKnob(name,clockPN, directionPN) \
void handleKnob_#name() {name.update();}\
RotaryKnob<clockPN, directionPN,#name> name; 
