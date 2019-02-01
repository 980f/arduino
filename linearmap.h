#pragma once //(C)2019 Andy Heilveil, github/980F

/** like arduino's map() but with subtle syntax and one axis with a fixed 0 as the low.*/
struct LinearMap {
  const unsigned top;
  const unsigned bottom;
  LinearMap(unsigned top, unsigned bottom = 0):
    top(top), bottom(bottom)  {
    //#done
  }

  /** convert fraction of range in 1.15 format to unsigned output without using floating point. */
  unsigned operator ()(AnalogValue avin)const {
    auto scaledup = long(top - bottom) * avin;
    unsigned reduced = bottom + ((scaledup + (1 << 14)) >> 15);
    //    dbg("\nLM:",top,"-",bottom, " in",avin, "\tup:", scaledup, "\tdone:", reduced);
    return reduced;
  }
};
