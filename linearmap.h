#pragma once //(C)2019 Andy Heilveil, github/980F

/** like arduino's map() but with subtle syntax and one axis with a fixed 0 as the low.*/
struct LinearMap {
  unsigned top;
  unsigned bottom;
  LinearMap(unsigned top, unsigned bottom = 0):
    top(top), bottom(bottom)  {
    //#done
  }
  unsigned operator ()(AnalogValue avin)const {
    if (top == bottom) {
      return 0;
    }
    if (top > bottom) {
      auto scaledup = long(top - bottom) * avin;
      unsigned reduced = bottom + ((scaledup + (1 << 14)) >> 15);
      //    dbg("\nLM:",top,"-",bottom, " in",avin, "\tup:", scaledup, "\tdone:", reduced);
      return reduced;
    } else {
      auto scaledup = long(bottom-top) * ~avin;
      unsigned reduced = top + ((scaledup + (1 << 14)) >> 15);
      //    dbg("\nLM:",top,"-",bottom, " in",avin, "\tup:", scaledup, "\tdone:", reduced);
      return reduced;
    }
  }
};
