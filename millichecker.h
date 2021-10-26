#pragma once

#include "millievent.h"

/** 
utility to see iif youare missing milliseconds in your loop().
call check() often, and then show when it has been a while.

todo: pass stream to print upon rather than hardcoding Serial.

 */
class MilliChecker {
  static const unsigned numhist = 100;
  unsigned millihist[numhist + 1]; //trusting zero init.
  unsigned millilast = 0;
public:
  void check() {
    unsigned delta = MilliTicked.since(millilast);
    ++ millihist[min(delta, numhist)];
    millilast = MilliTicked.recent();
  }

  void show(bool andClear=true) {
    Serial.println();//ensure alignment of output
    for (unsigned hi = numhist + 1; hi-- > 0;) {
      if (millihist[hi] > 0) {          
        Serial.print(hi); Serial.print(":\t"); Serial.println(millihist[hi]);
        if (andClear) {
          millihist[hi] = 0;
        }
      }
    }
  }
} ;
