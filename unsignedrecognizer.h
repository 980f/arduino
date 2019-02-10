#pragma once

#include "char.h"
/** recognize a sequence of digits. if asked for the value provide it, but clear it. You can peek at the value, but shouldn't except for debugging this module itself.
  leading zeroes are effectively ignored, they do not trigger octal interpretation. */
struct UnsignedRecognizer {
  unsigned accumulator = 0;

  /** inspect incoming character, @returns whether it is part of the number and if so had added it to local number.*/
  bool operator()(int key) {
    if (Char(key).appliedDigit(accumulator)) {
      return true;
    } else {
      return false;
    }
  }

  /** @returns whether the incooming number is !=0
   *  not using operator bool() as the compiler preferred to use operator unsigned() instead of oper bool(), so testing the value cleared it.
  */
  bool operator ~()const {
    return accumulator != 0;
  }

  /** look at it and it is gone! */
  operator unsigned() {
    return take(accumulator);
  }
};
