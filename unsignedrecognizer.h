#pragma once

#include "char.h"  //and to your build add: char.cpp and index.h and cstr.h and textkey.h 
/** recognize a sequence of digits. if asked for the value provide it, but clear it. You can peek at the value, but shouldn't except for debugging this module itself.
  leading zeroes are effectively ignored, they do not trigger octal interpretation. */


template<typename Unsigned=unsigned>
struct UnsignedRecognizer {
  Unsigned accumulator = 0;

  /** inspect incoming character, @returns whether it is part of the number and if so has added it to local number.*/
  bool operator()(char key) {
    if (Char(key).appliedDigit(accumulator)) {
    	return true;
    } else if(key=='~'){
    	accumulator = ~0; //to help debug some stuff, delete if it gets bothersome.
    	return true;
     } else if(key==8 || key ==0x7F){//bs or del
    	accumulator /= 10;
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
  operator Unsigned() {
    return take(accumulator);
  }
};
