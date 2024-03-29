#pragma once //(C) 2018 by Andy Heilveil, github/980F
#include <Arduino.h>     //for compilation outside arduino ide
#include "cheaptricks.h" //for changed()


//need user defines management in arduino! Until then change the following to 1 or 0, leave at 0 when pushing to git.
#if 0  
#warning "millievent timer debug enabled, will spew"
#include "chainprinter.h"
static ChainPrinter medbg(Serial, true);
#else
#define medbg(...)
#endif

/** SoftMilliTimer

  suggested use is to call this in loop() and if it returns true then do your once per millisecond stuff.
  void loop(){
    ...
    if(MilliTicker.ticked()){
      //can use MilliTicker.recent() where you might have used millis(), for performance and coherence.
      //don't print from here or call anything with delay() in it unless you want to skip milliseconds,
    }
    ...
  }


*/

using MilliTick = decltype(millis());//unsigned long; 32 bits, 49 days
const MilliTick BadTick = ~0;   //~0 is a neat way to get "max unsigned" a.k.a. all ones without knowing the exact type.


class SoftMilliTimer {
    MilliTick lastChecked = 0; //0: ticked() will not return true until at least one ms has expired after reset.
  public:
    /** true only when called in a different millisecond than it was last called in. */
    bool ticked() {
      return changed(lastChecked, millis());
    }

    operator bool() {
      return ticked();
    }

    /** most recent sampling of millis(). You should be biased to use this instead of rereading millis().*/
    MilliTick recent() const {
      return lastChecked;
    }

    /** @returns the time in the future that will be @param duration after 'now' */
    MilliTick operator[](MilliTick duration) {
      if (duration == BadTick) {
        return duration;
      }
      return duration + lastChecked;
    }

    /** alias for @see recent() */
    operator MilliTick() const {
      return recent();
    }

    /** ticks since someone recorded recent(). */
    unsigned since(MilliTick previous) {
      return unsigned(lastChecked - previous);
    }

    /** if called often enough to not miss any ticks then this will be true once every @param howoften calls.*/
    bool every(unsigned howoften) const {
      return (lastChecked % howoften) == 0;
    }

    /** test and 'clear' on a timer value */
    bool timerDone(MilliTick &timer) const {
      if (timer != BadTick && timer <= lastChecked) {
        medbg("MT:timer:", timer, " Done@", lastChecked);
        timer = BadTick;
        return true;
      } else {
        return false;
      }
    }

    /** @returns the time until the @param value will be exceeded. Zero is returned for both timer not active and a pathological case of millis never tested and timer never set for some future time */
    MilliTick remaining(MilliTick timer) const {
      return (timer == BadTick || timer <= lastChecked) ? 0 : timer - lastChecked;
    }

#ifdef medbg   //if it is a defined symbol then we have no logging. to make this work you MUST use auto pop=logging();
    static bool logging(bool enabled) {
      return false;
    };
#else
    static FlagStacker logging(bool enabled) {
      return medbg.stackStifled(!enabled);
    }
#endif

};

//only one is needed:
extern SoftMilliTimer MilliTicker;//name changed ~Jan20,2022 to force all users to review code due to significant changes and bug fixes.

//////////////////////////////////////////////////////////////////////////////////////////////////
/** simplest version of timing a single future event.
  MonoStable is meant for recurring events. */
class OneShot {
  protected://for debug access
    MilliTick timer = BadTick;
  public:
    void operator =(MilliTick duration) {
      timer = MilliTicker[duration];
      medbg("OS:op=", duration, " due at:", timer);
    }

    void stop() {
      medbg("OS:stop@", timer);
      timer = BadTick;
    }

    /** @returns true once after use of operator=(), when time is up */
    bool isDone() {
      medbg("OS:bool@", timer);
      return MilliTicker.timerDone(timer);
    }

    /** @returns @see isDone() */
    operator bool() {
      return isDone();
    }

    /** @returns whether this timer is enabled/ not expired. Unlike isDone() it does not indicate a transition  */
    bool isRunning() const {
      return timer != BadTick;
    }

    /** @returns amount of time left, is slightly misnamed */
    MilliTick due() const {
      return MilliTicker.remaining(timer);
    }

    /** @returns the absolute time that the timer will expire at, ~0 if not running. */
    MilliTick expiry() const {
      return timer;
    }

    /** constructing one of these stops the timer, destruction resumes where it left off, which is not going to be the original expiration time.*/
    class Holder {
        OneShot&oneshot;
        MilliTick held;
      public:
        Holder(OneShot&oneshot): oneshot(oneshot) {
          held = oneshot.due();
        }

        ~Holder() {
          oneshot = held;
        }


        /** if @param dumpit is true (default) then erase the remaining time such that after this object is destroyed the oneshot stays stopped */
        void discard(bool dumpit = true) {
          if (dumpit) {
            held = BadTick;
          }
        }


        Holder(Holder &&factoried): Holder(factoried.oneshot) {
          factoried.discard(true);
        }

    };

    Holder hold() {
      return Holder(*this);
    }

};

/** implements a repeatable, stretchable interval.
    configure via set(), for efficiency check in your loop()'s "if(MilliTicked){}" section (you have one already, right? ;))
*/
class MonoStable : public OneShot {
  protected:
    /** this class differs from OneShot in that it remembers how long it should be activew such that the point of restarting it doesn't need to know */
    MilliTick interval;
  public:
    /** combined create and set, if nothing to set then a default equivalent to 'never' is used.*/
    MonoStable(MilliTick duration = BadTick, bool andStart = true): interval(duration) {
      if (andStart) {
        start();
      }
    }
    /** sets duration, which you may change while running,
        @param andStart is whether to restart the timer as well, default yes.
        @returns prior duration.
    */
    MilliTick set(MilliTick duration, bool andStart = true) {
      MilliTick old = interval;
      interval = duration;
      medbg("MS:set:", interval);
      if (andStart) {
        start();
      }
      return old;
    }

    /** sugar for setting the duration and starting, i.e. trigger with new value.  */
    MonoStable &operator =(MilliTick duration) {
      medbg("MS:op=", duration);
      set(duration);
      return *this;
    }

    /** call to indicate running starts 'now', a.k.a. retriggerable monostable. */
    void start() {
      medbg("MS:start@", interval, " was@", timer);
      //gave up trying to get to base class op= when we also have an op=
      timer = MilliTicker[interval];
      medbg("MS:timer==", timer);
    }

    /** if @param please is true then if not running start else retain original expiration time. If please is false then stop now.
      @returns whether it is running, which should be equal to the input parameter unless interval has been set to BadTick, which it is on construction. */
    bool beRunning(bool please) {
      if (please) {
        if (!isRunning()) {
          start();
        }
      } else {
        stop();
      }
      return isRunning();
    }

    /** @returns set time, use set() to modify it. */
    operator MilliTick() const {
      return interval;
    }

    /** @returns whether time has expired, and if so restarts it.  */
    bool perCycle() {
      if (bool(*this)) {
        start();
        return true;
      } else {
        return false;
      }
    }

    /*time since start if not stopped. Can exceed programmed time if you haven't called isDone() */
    MilliTick elapsed() const {
      return interval - due();
    }

};

/** a timer that retriggers with alternating values, the core of a software PWM */
class BiStable : public OneShot {
    bool phase;
    MilliTick biphase[2];
  public:
    BiStable(MilliTick obduration = BadTick, MilliTick produration = BadTick, bool andStart = true):
      phase(0) {
      biphase[1] = obduration;
      biphase[0] = produration;
      if (andStart) {
        *this = biphase[phase];
      }
    }

    /** @returns true while in the active phase */
    operator bool() {
      return phase;
    }

    /** @returns true once per cycle */
    virtual bool perCycle() {
      if (*this) {//if phase is complete
        phase = !phase; //toggle it
        *this = biphase[phase];//starts next phase time
        return phase;//only report start of one of the phases.
      } else {
        return false;
      }
    }

};
