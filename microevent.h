#pragma once  //(C) 2018,2019 Andy Heilveil, github/980f
#include <Arduino.h> //added for avr build. curious how different the build is for different processors.
#include "cheaptricks.h" //for changed()

#ifdef MicroDebug 
#include "easyconsole.h"
static EasyConsole<decltype(MicroDebug)> udbg(MicroDebug, true /*autofeed*/);
#else
static void udbg(...) {}
#endif


/** a polled timer with microsecond range. similar to millievent stuff but couldn't be merged due to need for range extension.
    the addition of a 16 bit counter stretches the usable range from micros()'s 71 minutes to 8.9 years.
    if we use just a byte for the wraps then we would have 12.7 days. I am going to do that.
*/


using RawMicros = decltype(micros());
struct MicroTick {
    using Cycler = uint8_t; //8 bits for 12.7 days, 16 bits for 8.9 years
    /** what micros() returns */
    RawMicros ticks; //0: will not return true until at least one us has expired after reset.
    /** range extender*/
    Cycler wraps;
    /** internal use only. */
    MicroTick(RawMicros ticks, Cycler wraps): ticks(ticks), wraps(wraps) {
      //#done
    }
  public:
    void invalidate() {
      wraps = Cycler(~0);
      ticks = RawMicros(~0);
    }

    MicroTick() {
      invalidate();
    }

    /** @returns whether this ever been assigned to from anything which had ever been assigned to.
        NB: the existence of this cast operator makes the compiler emit a caution that it considered doing this->bool->int when overloading operator +
    */
    operator bool () const {
      return wraps != Cycler(~0) && ticks != RawMicros(~0);
    }


    /** @returns the value @param increment micros from this one. */
    MicroTick& operator +=(unsigned increment) {
      RawMicros newticks(ticks + increment);
      if (newticks < ticks) {//wrapped, in asm would add w/carry to wraps.
        ++wraps;
      }
      ticks = newticks;
      return *this;
    }

/** @returns the value @param increment micros from this one. */
    MicroTick& operator -=(unsigned increment) {
      RawMicros newticks(ticks - increment);
      if (newticks > ticks) {//wrapped, in asm would add w/carry to wraps.
        --wraps;
      }
      ticks = newticks;
      return *this;
    }

      /** @returns the value @param increment micros from this one. */
    MicroTick operator +(unsigned increment)const {
      MicroTick future(ticks + increment, wraps);
      if (future.ticks < ticks) {//wrapped, in asm would add w/carry to wraps.
        ++future.wraps;
      }
      return future;
    }

       /** @returns the value @param increment micros from this one. */
    MicroTick operator -(unsigned increment)const {
      MicroTick future(ticks - increment, wraps);
      if (future.ticks > ticks) {//wrapped, in asm would add w/carry to wraps.
        --future.wraps;
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
    bool refresh(RawMicros clock) {
      RawMicros was = ticks;
      if (changed(ticks, clock)) {//if we don't check for exactly 2^32 microseconds then this logic fails.
//~800us for the message      	udbg("rf:",was,',',clock);
        if (ticks < was) {//if you don't check often enough this logic will fail.
          ++wraps;
        }
        return true;
      } else {
        return false;
      }
    }

    /** @returns truncated to 32 bits, same as micros() system call.*/
    RawMicros us() const {
      return ticks;
    }

    /** debug access, shouldn't need to reference in application code. */
    Cycler rollovers() {
      return wraps;
    }
    /** rounded to nearest seconds. avoiding float to make program smaller if it otherwise didn't need float */
    uint32_t secs() const {
      uint64_t nofloat = wraps;
      nofloat <<= 32;
      nofloat += ticks;
      nofloat += 500000;
      nofloat /= 1000000;
      return nofloat;
    }


  public: //compare operators
    //in all of the compares we do NOT use a reference, as that would expose us to something being updated in an ISR. (actually there is no guarantee of an atomic push, so this is moot alh:20aug2019)
    //we also don't combine the simpler ones for the combined operations for performance reasons.
    bool operator ==(MicroTick other) const {
      return wraps == other.wraps && ticks == other.ticks;
    }

    bool operator >(MicroTick other) const {
      return wraps > other.wraps || ( wraps == other.wraps && ticks > other.ticks);
    }

    bool operator >=(MicroTick other) const {
      //#not cascading due to code space and time efficiency
      return wraps > other.wraps || ( wraps == other.wraps && ticks >= other.ticks);
    }

    bool operator <(MicroTick other) const {
      return wraps < other.wraps || ( wraps == other.wraps && ticks < other.ticks);
    }

    bool operator <=(MicroTick other) const {
      return wraps < other.wraps || ( wraps == other.wraps && ticks <= other.ticks);
    }
};

class SoftMicroTimer {
    MicroTick lastchecked;

  public:
    SoftMicroTimer() {
      lastchecked.ticks = micros();
      lastchecked.wraps = 0;
    };
    /** true only when called in a different tick than it was last called in. */
    operator bool() {
      return lastchecked.refresh(micros());
    }
    /** most recent sampling of micros(). You should be biased to use this instead of rereading micros() in a local scope.*/
    MicroTick recent() const {
      return lastchecked;
    }

    /** force check of micros().*/
    MicroTick now() {
      lastchecked.refresh(micros());
      return lastchecked;
    }


    bool timerDone(MicroTick &timer) const {
      if (timer && timer <= lastChecked) {
        timer.invalidate();
        return true;
      } else {
        return false;
      }
    }
    
    /** @returns time interval until timer will report 'done', 0 if it will never report 'done'*/
    MicroTick remaining(const MicroTick &timer) const {
      return timer && (timer > lastChecked) ? (timer - lastChecked) : 0;
    }



    /** using a shorter than MicroTick type for the increment as we only need precision for short intervals, use millievent if this doesn't have enough range. */
    MicroTick future(unsigned adder) const {
      return lastchecked + adder;
    };
};

//only one is needed:
extern SoftMicroTimer MicroTicked;


/** simplest version of timing a single future event.
  MicroStable is meant for recurring events. */
class MicroShot {
    MicroTick timer = 0;
  public:
    void operator =(MicroTick duration) {
      timer = MicroTicked.future(duration);
    }

    /** @returns true once after use of operator=(), when time is/has been up. invalidates timer when returning true */
    operator bool() {
      return MicroTicked.timerDone(timer);
    }

    /** @returns amount of time left, is slightly misnamed, 0 for 'will never be done' but also for 'is done but you haven't yet checked'  */
    MicroTick due() const {
      return MicroTicked.remaining(timer);
    }

    /** @returns whether timer is running, IE operator bool() will eventually return true (perhaps in the very far distant future) */
    bool isRunning()const {
      return bool(timer);//Note: bool of the time value is NOT the same as bool of this wrapper class of it.
    }

};


/** a retriggerable soft pulse
    if tested within an ISR then the foreground cannot call start() or stop() without disabling that ISR during the change.
    todo: flying changes should adjust expires when not restarting.
*/
class MicroStable {
  public:
    using Tick = RawMicros;

  public: //for diagnostic access
    MicroTick expires;
    Tick duration;
  public:
    /** combined create and set, if nothing to set then a default equivalent to 'never' is used.*/
    MicroStable(Tick duration = ~0, boolean andStart = true) {
      set(duration, andStart);
    }
    /** sets duration, which you may change while running,
        @param andStart is whether to restart the timer as well, default yes.
    */
    void set(Tick duration, boolean andStart = true) {
      if (andStart) {//stretches cycle in progress
        this->duration = duration;
        start();
      } else {
        if (isRunning()) {//adjust cycle, might become done.
          expires -= this->duration;
          expires += duration;
        }
        this->duration = duration;
        //and someone in the future may call start().
      }
    }

    void operator = (Tick duration) { //#non standard oper=
      set(duration);
    }

    /** call this to start it running 'now' restarting/extending time if already running, a.k.a. retriggerable monostable. */
    void start() {
      expires = MicroTicked.future(duration);
      udbg("\tue:",expires.ticks,",",expires.wraps);
    }

    /** stop it early. Will be neither running nor done.
        @returns whether timer was running when you called this.
    */
    bool stop() {
      bool wasRunning = isRunning();
      expires.invalidate();
      return wasRunning;
    }

    /** @returns whether timer has started and not expired== it has been at least 'done' since start() was called */
    bool isRunning() const {
      return expires && expires > MicroTicked.recent();
    }

    /** @returns whether time has expired, will be false if never started. */
    bool isDone() const {
      return MicroTicked.recent() >= expires;
    }

    /** @returns whether this is the first time called since became 'isDone', then alters object so that it will not return true again without another start.
          This is what 'isDone' should have been, but we aren't going to change that.
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
      	udbg('-');
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
