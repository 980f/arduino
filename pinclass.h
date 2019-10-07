#pragma clang diagnostic push
//like the Boolish classes we are creating objects which pretend to be booleans, so we ignore the following usually relevant warnings:
#pragma ide diagnostic ignored "cppcoreguidelines-c-copy-assignment-signature"
#pragma ide diagnostic ignored "google-explicit-constructor"
#pragma once
#include <Arduino.h> //some IDE's need this
#include "pinuser.h" //maple and some other guys deviated a bit on pin declaration syntax, this guy deals with that.
//#include "bitbanger.h" //BoolishRef
#include "boolish.h" // so that OutputPin may be passed to a generic bit flipping service
/* @param arduinoNumber is such as 13 for the typical LED pin.
  @param mode should be one of:: INPUT(0), INPUT_PULLUP(2), and OUTPUT(1)
  @param polarity is HIGH(1) if true means output pin high, LOW(0) if true means pin low.
  Via polarity you can add an inverting buffer to the hardware and change that in one place, the declaration,
  such as when driving an nFET to operate a higher voltage load where a HIGH on a pin is usually a LOW on the device.

  Since this is template code the compiler should generate the traditional line of arduino code, these should have no overhead whatsoever compared to using the digitalXXX calls explicitly.
  They allow for conditionally compiling in code which bypasses all the arduino indirection stuff and hits a hardware pin directly.

  The pinMode() command which is usually in the setup() gets called where declared.
  For statically declared pins (not inside a function) this executes shortly before setup() via the c-startup code.
  This means that statically declared Pins can be used in your setup() code, and subsequent declarations.

  If you declare a Pin in a function then the pinMode command executes each time you enter the function.

  The downside of a template based implementation is that each Pin is a class unto itself. That means you can't pass a reference to one to a routine such as a software serial port.
  There is a number() method that lets you pass the arduino number to standard arduino code, or to your code which creates a pin locally which does a potentially pointless pinMode() but otherwise works fine.

*/

template <PinNumberType arduinoNumber, PinModeType mode, unsigned polarity = HIGH> struct Pin {
  /** pretending to not know that HIGH=1 and LOW=0 ... constexpr should inline '1-active' at each place of use and not actually do a function call */
  static constexpr bool inverse(bool active) {
    return (HIGH + LOW) - active;
  }
  enum : PinNumberType {
    /** in case you have to interface with something that takes the digitalXXX number*/
    number = arduinoNumber,
    active = polarity,
    inactive = inverse(polarity)
  };

  Pin() {
    pinMode(number, mode);
  }

  /** derived classes have operator bool(), we don't do that here as some variants do something special on read and we don't want the cost of virtual functions. */
  bool get()const {
    return digitalRead(number) == active;
  }

  bool setto(bool value) const { //const is allowed as this operation doesn't change the code, only the real world pin
    digitalWrite(number, value ? active : inactive);
    return value;
  }

  bool setto(int value) const { //const is allowed as this operation doesn't change the code, only the real world pin
    return setto(value != 0);
  }

  /// the following are utility functions, not essential to this class.
  /** using 'flip' instead of toggle due to derived classes also deriving from BoolishRef which has a 'toggle' that works for this but may not be as fast */
  bool flip() const { //const is allowed as this operation doesn't change the code, only the real world pin
    return setto(inverse(get()));
  }

};

/* some convenience class names:
    Note that the InputPin uses pullup mode.
    Also note that some devices have more options such as pulldown, that arduino does not provide access to.
*/
template < PinNumberType arduinoNumber, unsigned polarity = HIGH, PinModeType puller = polarity ? INPUT_PULLDOWN : INPUT_PULLUP > struct InputPin : public Pin<arduinoNumber, puller, polarity>, public BoolishRef {
  using super = Pin<arduinoNumber, puller, polarity>;
  operator bool() const {
    return super::get();
  }

  //abused to change pullup state, ignores defined polarity
  bool operator=(bool on) const override {
    pinMode(super::number, on ? INPUT_PULLUP : INPUT_PULLDOWN);
    return on;
  }

};


template <PinNumberType arduinoNumber, unsigned polarity = HIGH> struct OutputPin: public Pin<arduinoNumber, OUTPUT, polarity>, public BoolishRef {
  using super = Pin<arduinoNumber, OUTPUT, polarity>;

  bool operator =(bool value) const override {// NOLINT(cppcoreguidelines-c-copy-assignment-signature,misc-unconventional-assign-operator)
    return super::setto(value);
  }

  //reading an output pin is ambiguous, it is not clear if you are reading the pin or the requested output value. most of the time that makes no difference so ...:
  operator bool() const override {// NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
    return super::get();
  }

  /** an active edge is one which ends up at the given polarity*/
  void edge(bool leading) const {
    if (leading) {
      super::setto(super::inverse(polarity));
      super::setto(polarity);
    } else {
      super::setto(polarity);
      super::setto(super::inverse(polarity));
    }
  }

};

/** to drive a pair of pins in tandem, for when you need more drive or there is some other reason they should always match*/
template <unsigned p1, unsigned p2>
struct DuplicateOutput: public BoolishRef {
  const OutputPin<p1> theone;
  const OutputPin<p2> theother;
  bool operator=(bool on) const override {
    //todo: consider disabling interrupts around the pair
    theone = on;
    theother = on;
    return on;
  }

  operator bool () const override {
    return theone;
  }

};

/** to drive a pair of pins in tandem, for when one is always the complement of the other.*/
template <unsigned ppin, unsigned npin>
struct ComplementaryOutput : public BoolishRef {
  const OutputPin<ppin, 1> nominal;
  const OutputPin<npin, 0> complement;
  bool operator=(bool on)const override {
    //todo: break before make etc
    nominal = on;
    complement = on;
    return on;
  }

  operator bool () const override {
    return nominal;
  }

  /** out of band signalling, neither 1 nor zero. */
  void oob(bool on = false) const {
    nominal = on;
    complement = !on;
  }

};

#pragma clang diagnostic pop
