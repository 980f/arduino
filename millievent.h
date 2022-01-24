#pragma once //(C) 2018 by Andy Heilveil, github/980F
#include <Arduino.h>     //for compilation outside arduino ide
#include "cheaptricks.h" //for changed()


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
const MilliTick BadTick = ~0;   //~0 is hacker trick for "max unsigned" a.k.a. all ones.


class SoftMilliTimer {
    MilliTick lastChecked = 0; //0: will not return true until at least one ms has expired after reset.
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
      if (timer >= lastChecked) {
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

};

//only one is needed:
extern SoftMilliTimer MilliTicker;//name changed ~Jan20,2022 to force all users to review code due to significant changes and bug fixes.

//////////////////////////////////////////////////////////////////////////////////////////////////
/** simplest version of timing a single future event.
  MonoStable is meant for recurring events. */
class OneShot {
    MilliTick timer = BadTick;
  public:
    void operator =(MilliTick duration) {
      timer = MilliTicker[duration];
    }

    /** @returns true once after use of operator=(), when time is up */
    operator bool() {
      return MilliTicker.timerDone(timer);
    }

    /** @returns amount of time left, is slightly misnamed */
    MilliTick due() const {
      return MilliTicker.remaining(timer);
    }

    /** @returns whether timer is running, IE operator bool() will eventually return true (perhaps in the very far distant future) */
    bool isRunning()const {
      return timer != BadTick;
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
      if (andStart) {
        start();
      }
      return old;
    }

    /** sugar for setting the duration and starting, i.e. trigger with new value.  */
    MonoStable &operator =(MilliTick duration) {
      set(duration);
      return *this;
    }

    /** call to indicate running starts 'now', a.k.a. retriggerable monostable. */
    void start() {
      OneShot(*this) = interval;
    }

    /** if @param please is true then if not running start else retain original expiration time. If please is false then stop now. */
    void beRunning(bool please) {
      if (please) {
        if (!isRunning()) {
          OneShot(*this) = interval;
        }
      } else {
        stop();
      }
    }

    void stop() {
      OneShot(*this) = BadTick;
    }

    /** @returns whether time has expired since the last start */
    bool isDone() const {
      return bool(*this);
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
