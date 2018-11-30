#pragma once
#include "millievent.h"

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
      for (unsigned hi = numhist + 1; hi-- > 0;) {
        if (millihist[hi] > 0) {
          Serial.println();
          Serial.print(hi); Serial.print(":\t"); Serial.println(millihist[hi]);
          if (andClear) {
            millihist[hi] = 0;
          }
        }
      }
    }
} ;
