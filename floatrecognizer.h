#pragma once

#include "char.h"
/** recognize a sequence of digits and decimal point, but not 'E' notation.
  We don't do 'E' notation as we have extensive support for that in a different library: safely/cppext.

	If asked for the value we provide it, but clear it.
	You can peek at the value, but shouldn't except for debugging this module itself.
  Leading zeroes are effectively ignored, they do not trigger octal interpretation. */

#include <cmath>

struct FloatRecognizer {
  int beforedp = 0;
  unsigned afterdp = 0;
  /** ~0 is an impossble number of trailing digits so we can use it for 'no dp seen'
       0 will be 'dp seen, but no digits entered'
       ~digitsafter is true if dp has been seen
       !~digitsafter is true when no dp has been seen (integer number)
  */
  unsigned digitsafter = ~0;

  void clear() {
    *this = {}; //magic to set fields to the initial values in declaration.
  }

  /** inspect incoming character, @returns whether it is part of the number and if so had added it to local number.*/
  bool operator()(char key) {
    if (Char(key).appliedDigit(~digitsafter ? afterdp : beforedp)) {
      if (~digitsafter) {
        ++digitsafter;
      }
      return true;
    }
    switch (key) {
      case '-'://negative sign
        if (beforedp || afterdp || !~digitsafter) {
          return false;
        }
        beforedp = - beforedp;
        return true;
      case '.':
        if (~digitsafter) { //not already seen one
          return false;
        }
        ++digitsafter;
        return true;

      case '~'://breakpoint trigger
        beforedp = ~0; //to help debug some stuff, delete if it gets bothersome.
        return true;
      case 8: case 0x7F: //bs or del
        if (~digitsafter) { //no d.p.
          beforedp /= 10;
        } else if (digitsafter) { //some digits after dp
          afterdp /= 10;
          --digitsafter;
        } else if (!digitsafter) { //dp but no digits
          --digitsafter;
        }
        return true;
      default:
        return false;
    }
  }

  /** @returns whether the incoming number is looks like no digits or dp were entered, IE "not a number" however 0.0 is a number */
  bool operator ~()const {
    return beforedp == 0 && !~digitsafter;
  }

  operator bool() const {
    return !~*this;//expressions like this are why some people despise C++
  }

  /** look at it and it is gone! */
  operator float() {
    float eff = beforedp;
    if (afterdp) { //no point on converting .00000 to a number
      float fract = afterdp;
      eff += fract * pow10(-digitsafter);
    }
    clear();
    return eff;
  }

  /** if only accepting an integer for an operand then this is faster than making a float and then converting back to the int started with*/
  int asInt() {
    int truncated = beforedp;
    clear();
    return truncated;
  }
};
