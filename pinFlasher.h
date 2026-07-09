
#include "millievent.h" //initially just getting TimerTick declaration, might use more of it later.
/** flash an led with a complicated pattern */
#include "dbgserial.h"

class Flasher {
  const static unsigned numSteps = 15;
  const unsigned ledPin;
  TimerTick step[numSteps];
  
  unsigned int ledStep = 0;             
  TimerTick stepDoneAt=0;

  /** whether to show the number of times the LED has been toggled, with each toggle */
 public:
  Flasher(unsigned pinNumber):ledPin(pinNumber){}

  bool showCount = false;
  unsigned count = 0;

  public:
    void setup(){
      pinMode(ledPin, OUTPUT);
      for (unsigned i=numSteps;i-->0;){
        step[i]= i>6 ? 0 : 100*(i+1);
      }
      stepDoneAt = millis() + step[ledStep]; //make the first be the same as all of the rest.
    }

    void loop(TimerTick currentMillis){
      if (currentMillis >= stepDoneAt) {
        stepDoneAt = currentMillis + step[ledStep];
        if(++ledStep >= numSteps){
          ledStep = 0;
          ++count;
          if(showCount){
            dbg(count);
          }
        }
        digitalWrite(ledPin, ledStep & 1);   
      }
    }

    void adjust(unsigned which, unsigned amount){
       if(which<numSteps){
        dbg("Setting step ",which, " to ",amount);
        step[which] = amount;     
      } else {
        dbg("Unreasonable step number, can't set step ",which," to ",amount);
      }
    }
  
  void dump(){
    dbg("Steps:");
      for(unsigned i=0;i<numSteps;++i){
        dbg(i,'\t',step[i]);
      }
  }

};
