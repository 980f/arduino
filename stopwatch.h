#ifndef STOPWATCH_H
#define STOPWATCH_H  //(C) 2017,2018 Andy Heilveil, github/980f

#include "Arduino.h"

/** POSIX versions uses time_t classes which 980F/safely wraps, we mimic that here.
  Note: we use double but AVR uses 32bit for that. */
struct Microseconds {
  unsigned long micros;
  
  operator unsigned long()const {
    return micros;
  }

  operator double()const {
    return double(micros) / 1e6;
  }


  Microseconds& operator = (unsigned long ticks) {
    micros = ticks;
    return *this;
  }

  Microseconds &operator = (double seconds) {
    micros = seconds * 1e6; //truncating!
    return *this;
  }


  Microseconds& operator -=(const Microseconds &lesser) {
    micros -= lesser.micros;
    return *this;
  }

  Microseconds operator -(const Microseconds &lesser) const {
    Microseconds diff;
    diff = *this;
    diff -= lesser;
    return diff;
  }

  Microseconds& operator +=(const Microseconds &lesser) {
    micros += lesser.micros;
    return *this;
  }

  Microseconds operator +(const Microseconds &lesser) const {
    Microseconds diff;
    diff = *this;
    diff += lesser;
    return diff;
  }
  /* this implementation is optimized for returns of 0 and 1 and presumes non-negative this and positive interval */
  unsigned modulated(const Microseconds &interval) {
    if (interval.isZero()) {
      return 0;//gigo
    }
    unsigned cycles = 0;
    //we use repeated subtraction to do a divide since most times we cycle 0 or 1.
    while (*this >= interval) {
      *this -= interval;
      ++cycles;
    }
    return cycles;
  }
  
  bool operator >(const Microseconds &that) const {
    return micros > that.micros;
  }

  bool operator >=(const Microseconds &that) const {
    return micros > that.micros ;
  }
  
  bool operator <(const Microseconds &that) const {
    return micros < that.micros;
  }

  bool operator ==(const Microseconds &that) const {
    return micros == that.micros;
  }

  Microseconds &atLeast(const Microseconds &other)  {
    if (*this < other) {
      *this = other;
    } else if (isNever()) {
      *this = other;
    }
    return *this;
  }

  Microseconds &atMost(const Microseconds &other) {
    if (isNever()) {
      *this = other;
    } else if (*this > other) {
      *this = other;
    }
    return *this;
  }

  Microseconds &Never() {
    micros = ~0;
    return *this;
  }

  bool isNever()const {
    return micros == ~0U;
  }

  bool isZero() const noexcept {
    return micros == 0;
  }


  /** differs from posix due to implementation of delay being non-interruptible */
  void sleep()const {
    delayMicroseconds(micros);
  }

};

using Timebase = Microseconds;
/** an interval timer using Arduino micros() */
class StopWatch {

  private:
    //bridge from original source which did a posix call here.
    void readit(Timebase &ts) {
      ts = micros();
    }
  protected:
    Timebase started;
    Timebase stopped;

    bool running;
  public:
    /** @param beRunning is whether to start timer upon construction. */
    StopWatch(bool beRunning = true);

    /** @returns elasped time and restarts interval. Use this for cyclic sampling. @param absolutely if not null gets the absolute time reading used in the returned value.*/
    Timebase roll(Timebase *absolutely = nullptr);
    /** use start and stop for non-periodic purposes*/
    void start();
    /** stops acquiring (if not already stopped) and @returns REFERENCE to stopped tracker. You probably want elapsed() or rollit(), this guy is only needed for some time critical timing situations */
    void stop();
    /** convenient for passing around 'timeout pending' state */
    bool isRunning() const;
    /** updates 'stop' if running then @returns time between start and stop as seconds. @param absolutely if not null gets the absolute time reading used in the returned value.*/
    Timebase elapsed(Timebase *absolutely = nullptr);

    /** make last 'elapsed' be a start, retroactively (without reading the system clock again.*/
    void rollit();
    /** @return seconds of absolute time of stop, or now if running*/
    Timebase absolute();

    /** @returns the number of cycles of frequency @param atHz that have @see elapsed() */
    unsigned cycles(double atHz, bool andRoll = true);
    /** @returns how many intervals have passed, and if andRoll sets start modulo interval */
    unsigned periods(Timebase interval, bool andRoll = true);
    /** @return last clock value sampled, either as absolute (time since program start) or since stopwatch.start()*/
    Timebase lastSnap(bool absolute = false) const;
    void lap(const StopWatch &other);
};

#endif // STOPWATCH_H
