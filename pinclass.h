#pragma once


#ifndef ARDUINO
//stubs to test compile on a desktop development system.
void pinMode(unsigned pinnum, unsigned PINMODE);
bool digitalRead(unsigned pinnum);
void digitalWrite(unsigned pinnum, bool);
#endif

#ifdef MAPLE
#error need to: #define WiringPinMode(x)  x
#else
#define WiringPinMode(x)  x
#endif


//some devices have active pulldowns, some don't
#ifndef INPUT_PULLDOWN
#define INPUT_PULLDOWN INPUT
#endif
/* @param arduinoNumber is such as 13 for the typical LED pin.
  @param mode should be one of:: INPUT(0), INPUT_PULLUP(2), and OUTPUT(1)
  @param polarity is HIGH(1) if true means output pin high, LOW(0) if true means pin low.
  Via polarity you can add an inverting buffer to the declaration and change that in one place.
  Also if driving a FET to operate a higher voltage load a HIGH on a pin is usually a LOW on the device.

  Since this is template code the compiler should generate the traditional line of arduino code, these should have no overhead whatsoever compared to using the digitalXXX calls explicitly.

  The pinMode() command which is usually in the setup() gets called where declared.
  For statically declared pins (not inside a function) this executes shortly before setup() via the c-startup code.
  This means that statically declared Pins can be used in your setup() code.
  If you declare a Pin in a function then the pinMode command executes each time you enter the function.

  The downside of a template based implementation is that each Pin is a class unto itself. That means you can't pass a reference to one to a routine such as a software serial port.
  There is a number() method that lets you pass the arduino number to standard arduino code, or to your code which creates a pin locally which does a potentially pointless pinMode() but otherwise works fine.

*/



template <unsigned arduinoNumber, unsigned mode, unsigned polarity = HIGH> struct Pin {
  /** pretending to not know that HIGH=1 and LOW=0 ... constexpr should inline '1-active' at each place of use and not actually do a function call */
  static constexpr bool inverse(bool active) {
    return (HIGH + LOW) - active;
  }
  enum Bits {
    /** in case you have to interface with something that takes the digitalXXX number*/
    number = arduinoNumber,
    active = polarity,
    inactive = inverse(polarity)
  };

  Pin() {
    pinMode(number, WiringPinMode(mode)); //MAPLE made us do this cast
  }

  /** derived classes have operator bool(), we don't do that here as some variants do something special on read and we don't want the cost of virtual functions. */
  bool get()const {
    return digitalRead(arduinoNumber) == active;
  }

  bool setto(bool value) const { //const is allowed as this operation doesn't change the code, only the real world pin
    digitalWrite(arduinoNumber, value ? active : inactive);
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
template <unsigned arduinoNumber, unsigned polarity = HIGH, unsigned puller= polarity?INPUT_PULLUP:INPUT_PULLDOWN> struct InputPin: public Pin<arduinoNumber, puller, polarity> {
  operator bool() const {
    return Pin<arduinoNumber, puller, polarity>::get();
  }
};

#include "boolish.h" // so that it may be passed to a generic bit flipping service
template <unsigned arduinoNumber, unsigned polarity = HIGH> struct OutputPin: public Pin<arduinoNumber, OUTPUT, polarity>, public BoolishRef {
  using super = Pin<arduinoNumber, OUTPUT, polarity>;

  bool operator =(bool value) const override {
    return super::setto(value);
  }

  //reading an output pin is ambiguous, it is not clear if you are reading the pin or the requested output value. most of the time that makes no difference so ...:
  operator bool() const {
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
