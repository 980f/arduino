#pragma once

#include "cheaptricks.h" //changed

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

using TickType = unsigned long ; //return type of millis(), can I use declspec to get that?
const TickType BadTick = ~0UL; //hacker trick for "max unsigned"

class SoftMilliTimer {
    TickType lastchecked = 0; //0: will not return true until at least one ms has expired after reset.
  public:
    /** true only when called in a different millisecond than it was last called in. */
    operator bool() {
      return changed(lastchecked, millis());
    }
    /** most recent sampling of millis(). You should be biased to use this instead of rereading millis().*/
    TickType recent() const {
      return lastchecked;
    }
};

//only one is needed:
SoftMilliTimer MilliTicked;

/** indicates an interval.
    configure via set() check in if(MilliTicked  ){}
*/
class MonoStable {
    TickType zero = BadTick;
    TickType done;
  public:
    /** combined create and set, if nothing to set then a default equivalent to 'never' is used.*/
    MonoStable(TickType duration = BadTick): done(duration) {
      //#done.
    }
    /** sets duration, which you may change while running,
        @param andStart is whether to restart the timer as well, default yes.
        @returns prior duration.
    */
    TickType set(TickType duration, boolean andStart = true) {
      TickType old = done;
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
      TickType now = MilliTicked.recent();
      return now > zero && done > (now - zero); //todo: debate whether either of these should have an '='
    }

    /** @returns whether time has expired, will be false if never started. */
    bool isDone() const {
      TickType now = MilliTicked.recent();
      return now > zero && done <= (now - zero); //todo: debate whether either of these should have an '='
    }

};
