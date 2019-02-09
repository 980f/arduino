#pragma once

#include "char.h"
/** recognize a sequence of digits. if asked for the sequence provide it, but clear it. You can peek at the value, but shouldn't except for debugging this module itself.
  leading zeroes are effectively ignored, they do not trigger octal interpretation. */
struct UnsignedRecognizer {
  unsigned accumulator = 0;
  bool operator()(int key) {
    if (Char(key).appliedDigit(accumulator)) { //rpn number entry
      //      Console("\n",accumulator);
      return true;
    } else {
      return false;
    }
  }
  /** look at it and it is gone! */
  operator unsigned() {
    return take(accumulator);
  }
};
