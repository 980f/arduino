#pragma once

#include "cheaptricks.h"

/** a polled timer */
class SoftMilliTimer {
  unsigned long lastchecked=0;
  public:
  /** true only when called in a different millisecond than it was last called in. */
  operator bool(){
    return changed(lastchecked,millis());
  }
  unsigned long recent() const {
    return lastchecked;
  }
};

SoftMilliTimer milliEvent;
