#pragma once

#include "char.h"  //and to your build add: char.cpp and index.h and cstr.h and textkey.h 
/** recognize a sequence of digits. if asked for the value provide it, but clear it. You can peek at the value, but shouldn't except for debugging this module itself.
  leading zeroes are effectively ignored, they do not trigger octal interpretation. */

#include "cheaptricks.h" //take();

template<typename Unsigned=unsigned>
struct UnsignedRecognizer {
  Unsigned accumulator = 0;
  bool hexly=false; //user can force this, but has to do it before each value entered.

  /** inspect incoming character, @returns whether it is part of the number, If so it has been incorporated into the local number.
    added '#' prefix for hexadecimal strings for entering rgb values for WS281x LED manipulation, 3 8bit numbers packed into 24 out of 32 bits.
    '~' sets accumulator to all 1's, but you can then add more decimal or hex digits. #~BEEF -> FFFFBEEF
  */
  bool operator()(const char key) {
    if(key=='#' && accumulator==0){//testing accumulator so as to not deal with reinterpreting digits already entered.
      hexly=true;
      return true;
    }

    if (hexly? Char(key).appliedNibble(accumulator) : Char(key).appliedDigit(accumulator)) {
    	return true;
    } 
    
    if(key=='~'){
    	accumulator = ~0; //to help debug some stuff, delete if it gets bothersome.
    	return true;
    } 
    
    if(key==8 || key ==0x7F){//bs or del
    	if(hexly){
        accumulator /=16;
      } else {
        accumulator /=10;
      }
    	return true;
    } 
    return false;    
  }

/** @returns whether the incoming number is 0.
   *  not using operator bool() as the compiler preferred to use operator unsigned() instead of oper bool(), so testing the value cleared it.
   * this replaced operator ~ in order to better match other library operator overloading habits, this is obscure enough as it is without being discordant with other uses.
  */
  bool operator !() const {
    return accumulator == 0;
  }

  /** look at it and it is gone! 
  * @returns the present value, setting the local value to zero.
  */
  operator Unsigned() {
    hexly=false;//reset base with each value extracted
    return take(accumulator);
  }
};
