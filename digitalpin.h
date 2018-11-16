
/** This variation of Pin wrapping (see pinclass.h) can be passed by reference, which includes being able to be put into an array.
 *  It generates more code at each point of use. An aggressively optimizing compiler might eliminate that extra code, 
 *  but for the template version of this functionality the compiler most likley will write minimal code without being asked nicely
    
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

class DigitalPin {
public:
  const unsigned number;
  const unsigned polarity;

DigitalPin(unsigned arduinoNumber,unsigned mode,unsigned polarity=HIGH):
  number(arduinoNumber),
  polarity(polarity){
    pinMode(arduinoNumber,mode);
  }
  
  operator bool()const{
    return digitalRead(number)==polarity;
  }
  
  bool operator =(bool value)const{
    digitalWrite(number , value ? polarity : HIGH+LOW-polarity);
    return value;
  }
/// the following are utility functions, not essential to this class
  bool toggle()const {
    *this = ! *this;
     return operator bool();//FYI you can call operator overloads as if they were normal functions.
  }
  
};

/* some convenience classes:
 *  Note that the InputPin used pullup mode, it is rare that that is not what you want. 
 *  Also note that some devices have more options such as pulldown, that arduino does not provide access to.
 */
class DigitalInput: public DigitalPin {
  public:
  DigitalInput(unsigned arduinoNumber,unsigned polarity=HIGH):
  DigitalPin(arduinoNumber, INPUT_PULLUP, polarity){}
};

class DigitalOutput: public DigitalPin {
  public:
  DigitalOutput(unsigned arduinoNumber,unsigned polarity=HIGH):
  DigitalPin(arduinoNumber, OUTPUT, polarity){}
};
