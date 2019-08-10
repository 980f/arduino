#pragma once  //(C) 2018,2019 Andy Heilveil, github/980f

#include "cheaptricks.h" //for changed()


#include "easyconsole.h"
static EasyConsole<decltype(Serial)> udbg(Serial,true /*autofeed*/);

/** a polled timer with microsecond range. similar to millievent stuff but couldn't be merged due to need for range extension.
    the addition of a 16 bit counter stretches the usable range from micros()'s 71 minutes to 8.9 years.
    if we use just a byte for the wraps then we would have 12.7 days. I am going to do that.
*/

struct MicroTick {
    using RawTick = unsigned long; //type of micros()
    using Cycler = uint8_t; //8 bits for 12.7 days, 16 bits for 8.9 years
    /** what micros() returns */
    RawTick micros; //0: will not return true until at least one us has expired after reset.
    /** range extender*/
    Cycler wraps;
    /** internal use only. */
    MicroTick(RawTick micros, Cycler wraps): micros(micros), wraps(wraps) {
      //#done
    }
  public:
    MicroTick(): micros(~0), wraps(~0) {
      //#done
    }

    /** @returns whether this ever been assigned to from anything which had ever been assigned to.
        NB: the existence of this cast operator makes the compiler emit a caution that it considered doing this->bool->int when overloading operator +
    */
    operator bool () const {
      return ~wraps!=0 && ~micros!=0;
    }

    /** @returns the value @param increment micros from this one. */
    MicroTick operator +(unsigned increment)const {
      MicroTick future(micros + increment, wraps);
      if (future.micros < micros) {//wrapped, in asm would add w/carry to wraps.
        ++future.wraps;
      }
      return future;
    }

    //    /** @returns the number of microseconds after @param other that this is. */
    //    int operator -(MicroTick other)const {
    //      int diff = micros - other.micros;
    //      if (diff < 0) { //either wrapped or overdue
    //        if (wraps == other.wraps) {
    //          return diff;
    //        }
    //        if (wraps > other.wraps) {
    //          if (wraps == 1 + other.wraps) {
    //            return -diff;
    //          }
    //          return INT_MAX;
    //        }
    //      }
    //    }

    /** update with presumed fresh reading of device clock. @returns whether that caused a change (else two clock reads were same instant- most likely you did something very strange)*/
    bool refresh(RawTick clock) {
      RawTick was = micros;
      if (changed(micros, clock)) {//if we don't check for exactly 2^32 microseconds then this logic fails.
        if (micros < was) {//if you don't check often enough this logic will fail.
          ++wraps;
        }
        return true;
      } else {
        return false;
      }
    }

    /** @returns truncated to 32 bits, same as micros() system call.*/
    uint32_t us() const {
      return micros;
    }

    /** debug access, shouldn't need to reference in application code. */
    Cycler rollovers() {
      return wraps;
    }
    /** rounded to nearest seconds */
    uint32_t secs() const {
      uint64_t nofloat = wraps;
      nofloat <<= 32;
      nofloat += micros;
      nofloat += 500000;
      nofloat /= 1000000;
      return nofloat;
    }


  public: //compare operators
    //in all of the compares we do NOT use a reference, as that would expose us to something being updated in an ISR.
    //we also don't combine the simpler ones for the combined operations for performance reasons.
    bool operator ==(MicroTick other) const {
      return wraps == other.wraps && micros == other.micros;
    }

    bool operator >(MicroTick other) const {
      return wraps > other.wraps || ( wraps == other.wraps && micros > other.micros);
    }

    bool operator >=(MicroTick other) const {
      //#not cascading due to code space and time efficiency
      return wraps > other.wraps || ( wraps == other.wraps && micros >= other.micros);
    }

    bool operator <(MicroTick other) const {
      return wraps < other.wraps || ( wraps == other.wraps && micros < other.micros);
    }

    bool operator <=(MicroTick other) const {
      return wraps < other.wraps || ( wraps == other.wraps && micros <= other.micros);
    }
};

class SoftMicroTimer {
    MicroTick lastchecked;

  public:
    SoftMicroTimer() {
      lastchecked.micros = micros();
      lastchecked.wraps = 0;
      udbg("ut start:",lastchecked.micros);
    };
    /** true only when called in a different tick than it was last called in. */
    operator bool() {    	
//    	udbg("ut bool:",lastchecked.micros);
      return lastchecked.refresh(micros());
    }
    /** most recent sampling of micros(). You should be biased to use this instead of rereading micros() in a local scope.*/
    MicroTick recent() const {
      return lastchecked;
    }

    /** force check of micros().*/
    MicroTick now() {
//    	udbg("ut now:",lastchecked.micros);
      lastchecked.refresh(micros());
      return lastchecked;
    }

    /** using a short type for the increment as we only need precision for short intervals, use millievent if this doesn't have enough range. */
    MicroTick future(unsigned adder) const {
      return lastchecked + adder;
    };
};

//only one is needed:
extern SoftMicroTimer MicroTicked;
//only one is needed:

//mention this in just one module:
#define Using_MicroTicker SoftMicroTimer MicroTicked;


/** a retriggerable soft pulse
    if tested within an ISR then the foreground cannot call start() or stop() without disabling that ISR during the change.
*/
class MicroStable {

    MicroTick expires;
    unsigned duration;
  public:
    /** combined create and set, if nothing to set then a default equivalent to 'never' is used.*/
    MicroStable(unsigned duration = ~0, boolean andStart = true) {
      set(duration, andStart);
    }
    /** sets duration, which you may change while running,
        @param andStart is whether to restart the timer as well, default yes.
        @returns prior duration.
    */
    void set(unsigned duration, boolean andStart = true) {
      this->duration = duration; //for restarts.

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

      /** @returns whether timer has started and not expired== it has been at least 'done' since start() was called */
    bool isRunning() const {
      MicroTick now = MicroTicked.recent();
      return expires && expires > now;
    }

    /** @returns whether time has expired, will be false if never started. */
    bool isDone() const {
      return MicroTicked.recent() >= expires;
    }

  /** @returns whether this is the first time called since became 'isDone', then alters object so that it will not return true again without another start.
     *  This is what 'isDone' should have been, but we aren't going to change that.
    */
    bool hasFinished() {
      if (isDone()) {
        stop();
        return true;
      } else {
        return false;
      }
    }

    /** @returns whether time has expired, and if so restarts it. */
    bool perCycle() {
      if (isDone()) {
//only got here every 20ms when debug is spewing:      	udbg("perCycle:",expires.micros);
        start();
        return true;
      } else {
        return false;
      }
    }

    operator bool() {
      return isDone();
    }

    MicroTick expected() {
      return expires;
    }
    //proving to be expensive, ignore for now.
    //    MicroTick remaining() {
    //      return expires - MicrotTicked.now();
    //    }

};
