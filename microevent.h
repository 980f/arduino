#pragma once  //(C) 2018,2019 Andy Heilveil, github/980f

#include "cheaptricks.h" //for changed()

/** a polled timer with microsecond range. similar to millievent stuff but couldn't be merged due to need for range extension.
*/

 struct MicroTick {

using RawTick=unsigned long;//type of micros()
   
        RawTick micros; //0: will not return true until at least one us has expired after reset.
        unsigned wraps;
 MicroTick(RawTick micros,unsigned wraps):micros(micros),wraps(wraps){
  
 }
  public:
  MicroTick():micros(~0UL),wraps(~0U){
//#done
}
 
/**is valid if it has ever been assigned to from anything which is valid.*/
operator bool () const {
  return ~wraps && ~micros;
}

 MicroTick operator +(RawTick increment)const {
MicroTick future(micros+increment,wraps);
if(future.micros<micros){
  ++future.wraps
}
return future;
}

   bool operator ==(MicroTick other){//#do NOT use const reference, in case item is updated in ISR, which is common. 
	return wraps==other.wraps && micros==other.micros;
   }
   bool operator >(MicroTick other){
return wraps>other.wraps ||( wraps==other.wraps && micros>other.micros);
   }
   bool operator >=(MicroTick other){
//#not cascading due to code space and time efficiency
return wraps>other.wraps ||( wraps==other.wraps && micros>=other.micros);
   }
  bool operator <(MicroTick other){
return wraps<other.wraps ||( wraps==other.wraps && micros<other.micros);
   }
   bool operator >=(MicroTick other){
//#not cascading due to code space and time efficiency
return wraps<other.wraps ||( wraps==other.wraps && micros<=other.micros);
   }
 };

class SoftMicroTimer {
   MicroTick lastchecked;

  public:
    SoftMicroTimer() {
lastchecked.micros= micros();
lastchecked.wraps=0;
    };
    /** true only when called in a different tick than it was last called in. */
    operator bool() {
      RawTick was = lastchecked.micros;
      if (changed(lastchecked.micros, micros())) {//if we don't check for exactly 2^32 microseconds then this logic fails.
        if (lastchecked.micros < was) {//if you don't check often enough this logic will fail.
          ++lastchecked.wraps;
        }
        return true;
      } else {
        return false;
      }
    }
    /** most recent sampling of micros(). You should be biased to use this instead of rereading micros() in a local scope.*/
    MicroTick recent() const {
      return lastchecked;
    }
   /** using a short type for the increment as we only need precision for short intervals, use millievent if this doesn't have enough range. */
MicroTick future(unsigned adder){
  return lastchecked+adder;
};

//only one is needed:
SoftMicroTimer MicroTicked;

/** a retriggerable soft pulse
*/
class MicroStable {
   
    MicroTick expires;
unsigned duration;
  public:
    /** combined create and set, if nothing to set then a default equivalent to 'never' is used.*/
    MonoStable(unsigned duration = BadTick, boolean andStart = true){
      set(duration,andStart);
    }
    /** sets duration, which you may change while running,
        @param andStart is whether to restart the timer as well, default yes.
        @returns prior duration.
    */
    void set(unsigned duration, boolean andStart = true) {
     this->duration=duration;//for restarts.
     
      if (andStart) {
        start();
      }
    
    }
    /** call to indicate running starts 'now', a.k.a. retriggerable monostable. */
    void start() {
       expires = MicroTicked.future(duration);
    }
/** stop it early, forgets last natural end.*/
    void stop() {
      expires = MicroTick();
    }

    /** @returns whether time has expired, will be false if never started. */
    bool isDone() const {
      MicroTick now = MicroTicked.recent();
      return now>=expires;
    }

    /** @returns whether time has expired, and if so restarts it. */
    bool perCycle() {
      if (isDone()) {
        start();
        return true;
      } else {
        return false;
      }
    }

    operator bool() {
      return isDone();
    }

};
