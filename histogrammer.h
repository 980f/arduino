#pragma once

/** allocates template param quanity of 0-based bins.
    call operator () to increment a bin
    call show to dump the data.
*/

class HistogrammerBase {
    const unsigned numhist;
    unsigned * const hist; //an unchanging pointer to values that change.
    const bool onebased;

  public:
    unsigned checks = 0; //public for user convenience
  public:
    HistogrammerBase (const unsigned numhist, unsigned *hist, bool onebased = false): numhist(numhist), hist(hist), onebased(onebased) {}

    /** call with each @param value being histogrammed
       @returns @param value being histogrammed, passes through the value so that you can histogram in the midst of receiving the value.
    */
    unsigned check(unsigned value) {
      ++checks;//used to decide that perhaps it is time to report on the content.
      ++hist[min(value, numhist)];//our template ensures that hist has places for both 0 and numhist, ie numhist+1 is allocated.
      return value;
    }

    /** format options for a histogram dump */
    struct ShowOptions {
      bool andClear = true;
      bool skipZeroes = true; //also do not output zeroes anywhere in histogram, not just leading ones which are unconditionally skipped.
      //todo: when we implement an oor channel  bool oors=false;
      unsigned modulus = 1000; //how often to dump results
      ShowOptions(bool andClear, bool skipZeroes, unsigned modulus): andClear(andClear), skipZeroes(skipZeroes), modulus(modulus) {}
    };

    /** display histogram content on @param dbg printer, using @opts to format the dump. */
    bool show(Print &dbg, const ShowOptions &opts) {
      unsigned nonzeroes = 0;//for not outputting anything if whole histogram is zero
      for (unsigned hi = numhist; hi -- > 0;) {//"hi": Histogram Index, not some sort of maximum value.
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

    /** check and periodically dump the histogram
       @@param value being histogrammed
       @param dbg for where to periodically show the value,
       @param opts are output listing formatting options, including the periodicity
       @returns whether printing occurred
    */
    bool operator ()(unsigned value, Print &dbg, const ShowOptions &opts) {
      check(value);
      if (opts.modulus && !(checks % opts.modulus)) {
        return show(dbg, opts);
      }
      return false;
    }

};

/** a histogrammer and the storage for it. template param qty is also the maximum value, all values above it are effectively truncated to it */
template <unsigned qty> struct Histogrammer: public HistogrammerBase {
  enum { numhist = qty + 1}; //+1 for 'outofrange'
  unsigned hist[numhist];

  Histogrammer(): HistogrammerBase(numhist, hist) {}
};
