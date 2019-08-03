#pragma once

/** TODO: use debug printer instead of direct access to Serial.
    This variation of Pin wrapping (see pinclass.h fpr te,[;tae version) can be passed by reference, which includes being able to be put into an array.
    It generates more code at each point of use. An aggressively optimizing compiler might eliminate that extra code,
    but for the template version of this functionality the compiler most likely will write minimal code without being asked nicely

   These objects can be static and const. The constructor will run before setup() is called, which is not formally supported but works on Leonardo and Due so
   it probably works on most processors. A failure may be subtle depending upon the processor support's choice of default values for pins.

   Typical usage:
   (typically after all your #includes)
   DigitalOutput LED(LED_BUILTIN);
   DigitalOutput slowFlash(8); //slower flash on digital 8
   DigitalInput button1(4,LOW); //button on digital 4
   DigitalOutput pressed(5);// indicate button press is seen
	...
   void loop(){
      unsigned long clock=millis();//read once, use many, gets rid of trivial phase shift.
      LED = (clock%1000)>250;//one second cycle, 250/1000 duty cycle.
      slowFlash = (clock%10000)<1789;//10 second cycle, rougly 18% duty cycle (note I changed the operator compared to LED, just for fun)
      pressed = button1;//compare to digitalWrite(5,digitalRead(4)?LOW:HIGH);
   }

*/

#include "pinuser.h"

//to get verbose debug info define DebugDigitalPin to be the name of the device you wish to Print to.
#ifdef DebugDigitalPin
#include "chainprinter.h"
ChainPrinter dpdbg(DebugDigitalPin);
#else
#define dpdbg(...)
#endif

class DigitalPin {
	public:
	
  public:
    const unsigned number;
    const unsigned polarity;

    explicit DigitalPin(PinNumberType arduinoNumber, PinModeType mode, unsigned polarity = HIGH): number(arduinoNumber), polarity(polarity) {
//      dpdbg("\ndigital construct ",arduinoNumber);
      pinMode(arduinoNumber, mode);
    }

	//disallowing copy construction, it interferes with using pin as a boolean, compiler wants to create a copy of the pin instead of assign the value of one pin to another.
    DigitalPin(DigitalPin &&any) = delete;
    DigitalPin(const DigitalPin &any) = delete;

  /**formal 'toggled' computation, just in case some vendor defines HIGH and LOW as other than 1 and 0 */
    static constexpr bool inverse(bool active) {
      return (HIGH + LOW) - active;
    }

/** read the pin. @returns true when pin is at configured polarity */
    operator bool() const {
    	dpdbg("\ndigital read ",number);
      return digitalRead(number) == polarity;
    }

};

/* some convenience classes:
    Note that the InputPin used pullup mode, it is rare that that is not what you want.
    Also note that some devices have more options such as pulldown, that arduino does not provide uniform access to. If we find something to conditionally compile upon then we will set the input to pull the opposite of 'polarity'
*/
class DigitalInput: public DigitalPin {
  public:
    DigitalInput(unsigned arduinoNumber, unsigned polarity = HIGH):  DigitalPin(arduinoNumber, INPUT, polarity) {}
};

class DigitalOutput: public DigitalPin {
  public:
    DigitalOutput(unsigned arduinoNumber, unsigned polarity = HIGH): DigitalPin(arduinoNumber, OUTPUT, polarity) {}
/** write the pin, applying configured polarity */
    bool operator =(bool value)const {
    	dpdbg("\ndigital write bool ",value);
      digitalWrite(number , value ? polarity : inverse(polarity));
      return value;
    }

    bool operator =(int nonzero)const {
      dpdbg("\ndigital write int ",nonzero);
      return operator =(nonzero != 0);
    }

    bool operator =(const DigitalPin &rhs)const {
      dpdbg("digital write from pin ",rhs.number);
      return operator =(bool(rhs));
    }

    bool operator =(DigitalPin &&rhs)const {
      dpdbg("digital write from &&pin ",rhs.number);
      return operator =(bool(rhs));
    }

  public:   /// the following are utility functions, not essential to this class
    bool toggle()const {
      *this = ! *this;
      return operator bool();//FYI you can call operator overloads as if they were normal functions.
    }
};
