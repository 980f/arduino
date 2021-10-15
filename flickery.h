#include "millievent.h"


/** fixed offtime, variable ontime. Extend and overload be() to propagate the flicker to a real world device.
  todo: implement table-of-pointers to register onTick methods.
*/
struct Flickery {
  bool ison;
  MonoStable duration;

  //these are used as MilliTicks, but since we want flicker we can use a shorter int to save on ram.
  unsigned maxtime, mintime , offtime;

  Flickery(unsigned maxtime, unsigned mintime = 50, unsigned offtime = 50): ison(true), mintime(mintime), maxtime(maxtime) {
    //#nada so that we can statically construct
  }

  virtual void setup() {
    be(true);//autostart
    duration = 750;//value chosen for debug
  }

  /** must be called just once per millisecond (or other timebase) tick */
  void onTick() {
    if (duration.hasFinished()) {
      be(!ison);//just toggle, less code and random is random.
      duration = ison ? random(mintime, maxtime) : offtime;
    }
  }

  virtual void be(bool on) {
    ison = on;
  }

  //to ease some testing we kill the timer then toggle bits manually
  void freeze() {
    duration.stop();
  }
};
