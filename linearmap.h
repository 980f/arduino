#pragma once //(C)2019 Andy Heilveil, github/980F

#include "analog.h"

/** like arduino's map() but with subtle syntax and one axis with a fixed 0 as the low.
    if AvoidFP is defined to anything other than 0 then we also laboriously use long instead of float to save ~1.5k of code. This could be made much more efficient if we were to indulge in some asm, work for the AVR family.
*/
struct LinearMap : public Printable {
  unsigned top;
  unsigned bottom;

  LinearMap(unsigned top, unsigned bottom = 0):
    top(top), bottom(bottom)  {
    //#done
  }

//add idiot checking to operaetor():
//    if (top > bottom) {
//      auto scaledup = long(top - bottom) * avin;
//      unsigned reduced = bottom + ((scaledup + (1 << 14)) >> 15);
//      //    dbg("\nLM:",top,"-",bottom, " in",avin, "\tup:", scaledup, "\tdone:", reduced);
//      return reduced;
//    } else {
//      auto scaledup = long(bottom-top) * ~avin;
//      unsigned reduced = top + ((scaledup + (1 << 14)) >> 15);
//      //    dbg("\nLM:",top,"-",bottom, " in",avin, "\tup:", scaledup, "\tdone:", reduced);
//      return reduced;
//    }

  /** convert fraction of range in 1.15 format to unsigned output without using floating point. */
  unsigned operator ()(AnalogValue avin, bool clipit = true)const {
    if (top == bottom) {
      return 0;
    }
#if AvoidFP
    auto scaledup = long(top - bottom) * ~avin;
    unsigned mapped = bottom + ((scaledup + (1 << 14)) >> 15);
#else
    float fract = ~avin;
    fract /= 32768.0; //hopefully compiler will just twiddle the exponent
    unsigned mapped = unsigned(fract * (top - bottom) + 0.5 + bottom);
#endif
    if (clipit) {
      return clipped(mapped);
    } else {
      return mapped;
    }
  }

  /** inverse map. @returns a number that when passed to operator() will give the number passed in. */
  AnalogValue operator /(unsigned scaled) const {
#if AvoidFP
    return AnalogValue(  ((long(scaled) - bottom) << 15) / long(top - bottom)); //#casting here is crucial
#else
    return AnalogValue( ((float(scaled) / (top - bottom)) + bottom) * 32768.0, 15); //todo:00 this is wrong! but I am in a hurry to get other stuff done!
#endif
  }

  unsigned clipped(unsigned raw)const {
    if (raw > top) {
      return top;
    }
    if (raw < bottom) {
      return bottom;
    }
    return raw;
  }

  bool clip(unsigned &raw)const {
    if (raw > top) {
      raw = top;
      return true;
    }
    if (raw < bottom) {
      raw = bottom;
      return true;
    }
    return false;
  }


  size_t printTo(Print& p) const {
    return p.print(top) + p.print(",") + p.print(bottom);//#this format is required for beholder init system
  }

};
