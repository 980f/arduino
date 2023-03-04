#pragma once  // //(C) 2017,2018,2019,2021 Andy Heilveil, github/980f

#include "Arduino.h"

/** a class common to use of millis() and micros() functions.
supporting only milliseconds and microseconds until such time as Arduino adds nanosecond facilities to their standard library.
 */

using TimerTick = decltype(millis());//unsigned long; 32 bits, 49 days for millis, 70 minutes for micros
static const TimerTick BadTick = ~0;   //"max unsigned", will not happen except a tick before rollover, and rollover of the system timer is dealt with elsewhere (usually by cycling power before it can get there ;)
	
struct Timesource {
	const bool IsMillis;//there are two arduino time sources, millis and micros
	
	const float convertToSeconds;
	const TimerTick convertFromSeconds;
	
	constexpr Timesource():IsMillis(IsMillis),
	 convertToSeconds(IsMillis? 1e-3F : 1e-6F),
	convertFromSeconds(IsMillis? 1000 : 1000000){}
	
	
  void sleep(TimerTick ticks) const {
  	if(IsMillis){
  	  delay(ticks);
  	} else {
    	delayMicroseconds(ticks);
    }
  }

  /** sets this object to the time since last program start. */
  TimerTick &snap() const {
  	if(IsMillis){
  	  return ::millis();
  	} else {
      return ::micros();
    }
  }
  
};

extern const Timesource MilliClock;//everyone uses millis, and we would like to have a default so we will declare that it exists.
extern const Timesource MicroClock;//we might make support for micros() conditional, for the smallest platforms

template <bool IsMillis> struct TimeValueBase {
/** this class exists to make sure that operations on the following value use the associated timer and scaling values */
  TimerTick ticks;
  
	/** to appease type picky situations such as print() functions */	  
  operator unsigned long()const {
    return ticks;
  }

	/** effectively override C++ type pickiness. */
  TimeValueBase& operator = (unsigned long ticks) {
    this->ticks = ticks;
    return *this;
  }
  
/** @returns this after making sure that it is at least as advanced as @param other */
  TimeValueBase &atLeast(const TimeValueBase &other)  {
    if (*this < other) {
      *this = other;
    } else if (isNever()) {
      *this = other;
    }
    return *this;
  }

/** @returns this after making sure that it is no more advanced than @param other */
  TimeValueBase &atMost(const TimeValueBase &other) {
    if (isNever()) {
      *this = other;
    } else if (*this > other) {
      *this = other;
    }
    return *this;
  }

/** @returns this after setting it to the marker value for 'never assigned' or 'not a valid value' */
  TimeValueBase &Never() {
    ticks = BadTick;
    return *this;
  }

/** @returns whether this is the marker for 'never assigned' */
  bool isNever()const {
    return ticks == BadTick;
  }

/** @returns whethe this value is zero */
  bool isZero() const noexcept {
    return ticks == 0;
  }

  
  
  /** @returns quotient of this over @param interval leaving this the remainder. 
  One of the frequent uses ot timers is for cyclic behavior, this returns the number of cycles that have expired since the last call, leaving this the number of ticks since the last cycle.
  The implementation is optimized for returns of 0 and 1 as you should be calling it often enough to not miss cycles (for typical uses) */
  unsigned modulated(const TimeValueBase &interval) const {
    if (interval.isZero()) {
      return 0;//gigo
    }
    if(isNever){
      return 0;//somewhat gigo. Without this test we would get large numbers.
    }
    unsigned cycles = 0;
    //we use repeated subtraction to do a divide since most times we cycle 0 or 1.
    while (*this >= interval) {
      *this -= interval;
      ++cycles;
    }
    return cycles;
  }
  
public: //conversion to and from seconds. partial ticks is not a supported concept, any float value is deemed to be seconds, not timer ticks.
	/** if you force this to be seen as a float you will get seconds regardless of whether 'this' is ms or us. */
  operator float() const {
    return float(ticks) * IsMillis?MilliClock.convertToSeconds:MicroClock.converToSeconds;
  }

	
	/** if you are computing ticks then explicitly convert to integer as this module can't know whether to round() or ceil() or floor() for your partial tick */
  TimeValueBase &operator = (float seconds) {
    ticks = seconds * IsMillis?MilliClock.convertFromSeconds:MicroClock.convertFromSeconds;  //ceil()
    return *this;
  }

public: //now make it easy to do math without gratuitous type mismatch warnings

  TimeValueBase& operator -=(const TimeValueBase<IsMillis> &lesser) {
    ticks -= lesser.ticks;
    return *this;
  }

  TimeValueBase operator -(const TimeValueBase<IsMillis> &lesser) const {
    Microseconds diff;
    diff = *this;
    diff -= lesser;
    return diff;
  }

  TimeValueBase& operator +=(const TimeValueBase<IsMillis> &lesser) {
    ticks += lesser.ticks;
    return *this;
  }

  TimeValueBase operator +(const TimeValueBase<IsMillis> &lesser) const {
    Microseconds diff;
    diff = *this;
    diff += lesser;
    return diff;
  }
  
  //on the compares: we are not quite ready to commit to a compiler version that knows about <=> operator:
  
  bool operator >(const Timebase<IsMillis> &that) const {
    return ticks > that.ticks;
  }

  bool operator >=(const Timebase<IsMillis> &that) const {
    return ticks >= that.ticks ;
  }
  
  bool operator <(const Timebase<IsMillis> &that) const {
    return ticks < that.ticks;
  }
  
  bool operator <=(const Timebase<IsMillis> &that) const {
    return ticks <= that.ticks;
  }

  bool operator ==(const Timebase<IsMillis> &that) const {
    return ticks == that.ticks;
  }

};

using MilliTick = struct TimeValueBase<true>;
using MicroTick = struct TimeValueBase<false>;


