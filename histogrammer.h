#pragma once

/** allocates tempalte param quanity of 0-based bins.
    call operator () to increment a bin
    call show to dump the data.
*/

template <unsigned qty> class HistoGrammer {
  unsigned hist[qty + 1]; //+1 for 'invalid'
public:
  unsigned operator()(unsigned value) {
    ++hist[min(value, qty)];
    return value;
  }

  void show(bool skipZeroes=true,bool andClear=true) {
    Serial.println();//ensure alignment of output
    for (unsigned hi = qty + 1; hi-- > 0;) {
      if (!skipZeroes || hist[hi] > 0) {
        Serial.print(hi); Serial.print(":\t"); Serial.println(hist[hi]);
        if (andClear) {
          hist[hi] = 0;
        }
      }
    }
  }
} ;
