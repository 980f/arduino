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
#include "millievent.h" //for common types

class SoftMicroTimer {
    union {
      struct {//todo: conditional order on bigendian
        TickType lastchecked; //0: will not return true until at least one us has expired after reset.
        unsigned wraps;
      } s;
      uint64_t l;
    } t;

  public:
    SoftMicroTimer() {
      t.l = 0;
    };
    /** true only when called in a different millisecond than it was last called in. */
    operator bool() {
      TickType was = t.s.lastchecked;
      if (changed(t.s.lastchecked, micros())) {//if we don't check for exactly 2^32 microseconds then this logic fails.
        if (t.s.lastchecked < was) {
          ++t.s.wraps;
        }
        return true;
      } else {
        return false;
      }
    }
    /** most recent sampling of micros(). You should be biased to use this instead of rereading micros().*/
    TickType recent() const {
      return t.s.lastchecked;
    }
    uint64_t elapsed() const {
      return t.l;
    }
};

//only one is needed:
SoftMicroTimer MicroTicked;
