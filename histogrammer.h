#pragma once

/** allocates template param quanity of 0-based bins.
    call operator () to increment a bin
    call show to dump the data.
*/

class HistogrammerBase {
    const unsigned numhist;
    const bool onebased;
    unsigned * const hist;
  public:
    HistogrammerBase (const unsigned numhist, unsigned *hist, bool onebased = false): numhist(numhist), hist(hist), onebased(onebased) {}
    unsigned checks = 0; //user convenience
    unsigned check(unsigned value) {
      ++checks;
      ++hist[min(value, numhist)];
      return value;
    }

    struct ShowOptions {
      bool andClear = true;
      bool skipZeroes = true; //not just leading, always skips leading zeroes
      //todo: when we implement an oor channel  bool oors=false;
      unsigned modulus = 1000;
    };

    bool show(Print &dbg, const ShowOptions &opts) {
      unsigned nonzeroes = 0;//for not outputting anything if whole histogram is zero
      for (unsigned hi = numhist; hi -- > 0;) {
        bool isZero = hist[hi] == 0;
        if (opts.skipZeroes && isZero) {
          continue;
        }
        if (nonzeroes == 0 && isZero) { //zeroes are usually very common at the high end of the range so we drop all as a convenient way of dropping those.
          continue;  //don't count 'leading' zeroes.
        }
        if (!isZero || !opts.skipZeroes) {
          //if 1 (if one based)  is the first non-zero entry then print nothing
          if (hi > onebased || nonzeroes > 0) {
            if (nonzeroes++ == 0) {
              dbg.println();//ensure alignment of output, but only if we are definitely outputing something.
            }
            dbg.print(hi); dbg.print(":\t"); dbg.println(hist[hi]);
          }
          if (opts.andClear) {
            hist[hi] = 0;
          }
        }
      }
      return nonzeroes != 0;
    }

    bool operator ()(unsigned value, Print &dbg, const ShowOptions &opts) {
      check(value);
      if (!(checks % opts.modulus)) {
        return show(dbg, opts);
      }
      return false;
    }

};

template <unsigned qty> struct Histogrammer: public HistogrammerBase {
  enum { numhist = qty + 1};
  unsigned hist[numhist]; //+1 for 'outofrange'

  Histogrammer(): HistogrammerBase(numhist, hist) {}
};
