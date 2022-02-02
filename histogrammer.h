#pragma once

/** allocates template param quanity of 0-based bins.
    call operator () to increment a bin
    call show to dump the data.
*/

template <unsigned qty> class HistoGrammer {
    unsigned hist[qty + 1]; //+1 for 'outofrange'
    bool onebased = false;
  public:
    unsigned operator()(unsigned value) {
      ++hist[min(value, qty)];
      return value;
    }

    struct ShowOptions {
      bool andClear = true;
      bool skipZeroes = true; //not just leading, always skips leading zeroes
      //todo: when we implement an oor channel  bool oors=false;
    };

    void show(Print &dbg, const ShowOptions &opts) {
      unsigned nonzeroes = 0;//for not outputting anything if whole histogram is zero
      for (unsigned hi = numhist; hi -- > 0;) {
        bool isZero = hist[hi] == 0;
        if (skipZeroes && isZero) {
          continue;
        }
        if (nonzeroes == 0 && isZero) { //zeroes are usually very common at the high end of the range so we drop all as a convenient way of dropping those.
          continue;  //don't count 'leading' zeroes.
        }
        if (!isZero || !skipZeroes) {
          //if 1 (if one based)  is the first non-zero entry then print nothing
          if (hi > onebased || nonzeroes > 0) {
            if (nonzeroes++ == 0) {
              dbg.println();//ensure alignment of output, but only if we are definitely outputing something.
            }
            dbg.print(hi); dbg.print(":\t"); dbg.println(hist[hi]);
          }
          if (andClear) {
            hist[hi] = 0;
          }
        }
      }
      return nonzeroes != 0;
    }

    bool operator ()(Print &dbg, unsigned modulus, const ShowOptions &forShow) {
      check();
      if (!(checks % modulus)) {
        return show(dbg, forShow);
      }
      return false;
    }

} ;
