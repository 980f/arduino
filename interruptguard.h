#pragma once


/** if you must disable all interrupts for a bit, this class makes sure they go back on. 
 *  creating one of these in a function (or blockscope) disables interrupts until said function (or blockscope) exits.
 *  By using this fanciness you can't accidentally leave interrupts disabled.
 NB: is you don't name the object the compiler will immediately destruct it defeating the purpose of this class. See the LOCK macro.
 */
class CriticalSection {
  static volatile unsigned nesting;
public:
  CriticalSection(void){
    if(!nesting++){
      noInterrupts();
    }
  }

  ~CriticalSection (void){
    if(nesting) { //then interrupts were globally disabled by us
      if(--nesting == 0) {
        interrupts();
      }
    }
  }
};

//this does not allow for static locking, only for within a function's execution (which is a good thing!):
#define LOCK(somename) CriticalSection somename ## _locker

