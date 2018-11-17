#pragma once


#ifdef ARDUINO_MAPLE_RET6
//patch around DUE addition not present in Maple lib:
constexpr uint8 digitalPinToInterrupt(unsigned arduinoNumber){
  return arduinoNumber;
}
#endif

/**
Management of per pin interrupts.

These can be created static and const. 

To enable an interrupt routine assign a 1 to this object, a 0 to disable it.
Suggest naming is:

InterruptPin<button1Handler,6,FALLING> buttonInterrupts;//notice the pluralization, setting to 1 enables an indefinite number of interrupts.

style is one of:
    LOW to trigger the interrupt while the pin is low,
    CHANGE to trigger the interrupt whenever the pin changes value
    RISING to trigger when the pin goes from low to high,
    FALLING for when the pin goes from high to low. 
The Due board allows also:
    HIGH to trigger the interrupt while the pin is high. 

*/

using Isr = void(void);

template <Isr isr, unsigned pinNumber, unsigned style=FALLING>  struct InterruptPin {
  /** call the isr from regular code, occasionally useful, but @see attach() */
  void invoke(){
    isr();
  }
  /** If isr is to be always on then put pin=1 in setup().
      Invoking the isr once without an interrupt is often useful to deal with an interrupt possibly having occurred while the program was restarting. */
  void attach(bool andInvoke=false)const{
    attachInterrupt(digitalPinToInterrupt(pinNumber), isr, ExtIntTriggerMode(style));
    if(andInvoke){
      invoke();
    }
  }

  /** no provision for temporary disabling, so use this then call attach() again later. */
  void detach()const{
    detachInterrupt(digitalPinToInterrupt(pinNumber)); 
  }

  //this always works, however most chips have a seperate bit for doing this, so ... we code enable/disable as an assignment to a bit:
  void operator=(bool enable)const{
    if(enable){
      attach(false);
    } else {
      detach();
    }
  }

};


/**
 @see InterruptPin class, this is a non-template version. 
The differences are subtle, mostly involving the number of bytes of program consumed and whether you can pass a pointer to one.
This one could expose its innards and then be dynamically configured, only really useful if you have a chunk of logic that is dynamically reconfigured as 
to which pins it uses.
*/
class PinInterrupt {
  Isr isr;
  unsigned pinNumber;
  unsigned style;
public:
  PinInterrupt(Isr isr, unsigned pinNumber, unsigned style):isr(isr),pinNumber(pinNumber),style(style){
   //#done. Don't enable until setup() and then only if user wants to.
  }

 /** call the isr from regular code, occasionally useful, but @see attach() */
  void invoke(){
    isr();
  }

  /** If isr is to be always on then put pin=1 in setup().
      Invoking the isr once without an interrupt is often useful to deal with an interrupt possibly having occurred while the program was restarting. */
  void attach(bool andInvoke=false)const{
    attachInterrupt(digitalPinToInterrupt(pinNumber), isr, ExtIntTriggerMode(style));
    if(andInvoke){
      isr();
    }
  }

  /** no provision for temporary disabling, so use this then call attach() again later. */
  void detach()const{
    detachInterrupt(digitalPinToInterrupt(pinNumber)); 
  }

  //this always works, however most chips have a seperate bit for doing this, so ... we code enable/disable as an assignment to a bit:
  void operator=(bool enable)const{
    if(enable){
      attach(false);
    } else {
      detach();
    }
  }

};



