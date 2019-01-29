#pragma once

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
const MilliTick BadTick = ~0UL; //hacker trick for "max unsigned"

class SoftMilliTimer {
    MilliTick lastchecked = 0; //0: will not return true until at least one ms has expired after reset.
  public:
    /** true only when called in a different millisecond than it was last called in. */
    operator bool() {
      return changed(lastchecked, millis());
    }
    /** most recent sampling of millis(). You should be biased to use this instead of rereading millis().*/
    MilliTick recent() const {
      return lastchecked;
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
SoftMilliTimer MilliTicked;

/** indicates an interval.
    configure via set() check in if(MilliTicked  ){}
*/
class MonoStable {
    MilliTick zero = BadTick;
    MilliTick done;
  public:
    /** combined create and set, if nothing to set then a default equivalent to 'never' is used.*/
    MonoStable(MilliTick duration = BadTick, boolean andStart = true): done(duration) {
      if (andStart) {
        start();
      }
    }
    /** sets duration, which you may change while running,
        @param andStart is whether to restart the timer as well, default yes.
        @returns prior duration.
    */
    MilliTick set(MilliTick duration, boolean andStart = true) {
      MilliTick old = done;
      done = duration;
      if (andStart) {
        start();
      }
      return old;
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
      return perCycle();
    }

    MilliTick due() const {
      return done + zero;
    }

};
