#pragma once
#include <Arduino.h>
#include "cheaptricks.h" //for changed()

/** a polled timer.
  suggested use is to call this in loop() and if it returns true then do your once per millisecond stuff.
  void loop(){
    ...
    if(MilliTicked){
      //can use MilliTicked.recent() where you might have used millis(), for performance and coherence.
      //don't print from here unless you want to skip milliseconds, or call anything with delay() in it.
    }
    ...
  }
*/

using MilliTick = unsigned long ; //return type of millis(), can I use declspec to get that?
const MilliTick BadTick = ~0UL;   //hacker trick for "max unsigned"

class SoftMilliTimer {
    MilliTick lastchecked = 0; //0: will not return true until at least one ms has expired after reset.
  public:
    /** true only when called in a different millisecond than it was last called in. */
    
        /** true only when called in a different millisecond than it was last called in. */
    bool ticked() {
      return changed(lastchecked, millis());
    }
    
    operator bool() {
      return ticked();
    }
    
    /** most recent sampling of millis(). You should be biased to use this instead of rereading millis().*/
    MilliTick recent() const {
      return lastchecked;
    }

    /** alias for @see recent() */
    operator MilliTick() const {
      return recent();
    }

    /** ticks since someone recorded recent(). */
    unsigned since(MilliTick previous) {
      return unsigned(lastchecked - previous);
    }

    /** if called often enough to not miss any ticks then this will be true once every @param howoften calls.*/
    bool every(unsigned howoften) const {
      return (lastchecked % howoften) == 0;
    }
};

//only one is needed:
extern SoftMilliTimer MilliTicked;

/** indicates an interval.
    configure via set(), for efficiency check in if(MilliTicked){}
*/
class MonoStable {
  protected:
    MilliTick zero = BadTick;//this choice ensures that both isDone and isRunning are false until an real cycle has at least begun.
    MilliTick done; //time after zero at which the timer is done.
  public:
    /** combined create and set, if nothing to set then a default equivalent to 'never' is used.*/
    MonoStable(MilliTick duration = BadTick, bool andStart = true): done(duration) {
      if (andStart) {
        start();
      }
    }
    /** sets duration, which you may change while running,
        @param andStart is whether to restart the timer as well, default yes.
        @returns prior duration.
    */
    MilliTick set(MilliTick duration, bool andStart = true) {
      MilliTick old = done;
      done = duration;
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
      zero = MilliTicked.recent();
    }

    void stop() {
      zero = BadTick;
    }

    /** @returns whether timer has started and not expired== it has been at least 'done' since start() was called */
    bool isRunning() const {
      MilliTick now = MilliTicked.recent();
      return now > zero && done > (now - zero);
    }

    /** @returns whether time has expired, will be false if never started. */
    bool isDone() const {
      MilliTick now = MilliTicked.recent();
      return now > zero && done <= (now - zero);
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

    /** sugar for isDone() */
    operator bool() const {
      return isDone();
    }

    /** @returns set time, use set() to modify it. */
    operator MilliTick() const {
      return done;
    }

    /** @returns whether time has expired, and if so restarts it.
        made virtual for BiStable
    */
    virtual bool perCycle() {
      if (isDone()) {
        start();
        return true;
      } else {
        return false;
      }
    }

    /** @return when it will be done, which can be in the past if already done.*/
    MilliTick due() const {
      return done + zero;
    }

};

/** a monostable that retriggers with alternating values */
class BiStable : public MonoStable {
    bool phase;
    MilliTick biphase[2];
  public:
    BiStable(MilliTick obduration = BadTick, MilliTick produration = BadTick, bool andStart = true):
      MonoStable(produration, andStart),
      phase(0) {
      biphase[1] = obduration;
      biphase[0] = produration;
    }

    /** sugar for isDone() */
    operator bool() {
      return phase;
    }

    virtual bool perCycle() {
      if (MonoStable::isDone()) {
        phase = !phase;
        done = biphase[phase];
        start();
        return true;
      } else {
        return false;
      }
    }

};
