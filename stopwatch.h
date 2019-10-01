#ifndef STOPWATCH_H
#define STOPWATCH_H  //(C) 2017,2018 Andy Heilveil, github/980f

//todo: template the microseconds class with an ulp and a function pointer to get the current time.
#include "microseconds.h"

using Timebase = Microseconds;
/** an interval timer using Arduino micros() */
class StopWatch {

  private:
    //bridge from original source which did a posix call here.
    void readit(Timebase &ts) {
      ts.snap();
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
