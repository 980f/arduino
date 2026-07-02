#pragma once

#include "char.h"  //and to your build also add: char.cpp and index.h and cstr.h and textkey.h 
/** recognize a sequence of digits. if asked for the value provide it, but clear it. You can peek at the value, but shouldn't except for debugging this module itself.
  leading zeroes are effectively ignored, they do not trigger octal interpretation. */

template<typename Unsigned = unsigned>
struct UnsignedRecognizer {
  Unsigned accumulator = 0;
  bool explicitlyZero = false; //whether a zero char has been entered but no other chars. Added to distinguish 'no entry' from zero 
  bool hexly = false; //user can force this, but has to do it before each value entered.

  /** inspect incoming character, @returns whether it is part of the number, If so it has been incorporated into the local number.
    added '#' prefix for hexadecimal strings for entering rgb values for WS281x LED manipulation, 3 8bit numbers packed into 24 out of 32 bits.
    '~' sets accumulator to all 1's, but you can then add more decimal or hex digits. #~BEEF -> FFFFBEEF
  */
  bool operator()(const char key) {
    if (key == '#') { 
      //testing accumulator so as to not deal with reinterpreting digits already entered.
      if(accumulator == 0 && !explicitlyZero){
        hexly = true;
        return true;
      } else {
        return false;
      }      
    }

    if (hexly ? Char(key).appliedNibble(accumulator) : Char(key).appliedDigit(accumulator)) {
      explicitlyZero = accumulator==0;
      return true;
    }

    if (key == '~') { //to help debug some stuff, delete this key handler it gets bothersome.
      accumulator = ~0; 
      explicitlyZero = false;
      return true;
    }

    if (key == 8 || key == 0x7F) { //bs or del
      if (hexly) {
        accumulator /= 16;
      } else {
        accumulator /= 10;
      }
      //but we don't update 'explicitlyZero'
      return true;
    }
    return false;
  }

  void clear() {
    hexly = false;
    accumulator = 0;
    explicitlyZero = false;
  }

  /** this is a replacement for the operator ! which seemed to occasionally not be used by the compiler over applying ! to operator Unsigned(). 
    @returns whether some decimal chars were passed to this guy since the last clear() */
  bool notEmpty() const {
    return accumulator != 0 || explicitlyZero;
  }

  /** look at it and it is gone!
    @returns the present value, setting the local value to zero.
  */
  operator Unsigned() {
    Unsigned value=accumulator;
    clear();
    return value;
  }
};
