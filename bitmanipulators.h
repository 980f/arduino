//
// Copyright 2021 by Andy Heilveil (github/980f) created on 12/20/21.
//

#pragma once


/** yet another variant of manipulating bits.
This variant was written for the fixedpoint.h class which in turn was written for factoring (and fixing) the vl53l0x library from ST micro. 
Do not muck with signatures, the operand order was chosen to make it easy to rewrite the mentioned library.
*/

#if __has_include(<cstdint>)
#include <cstdint>  //uint8_t in a few places
#else
#include "stdint.h"
#endif

#include "intsizer.h" //computes the type needed to hold a particular whole number.

/** bitmask of a given bit in a given type.   
 * strange name as too many bit() functions are flying around at the moment, should rename once we get rid of such
 * */
template<typename Intish=uint8_t> constexpr Intish Bitter(unsigned bitnum) {
  return Intish(1) << bitnum;
}

/** annoying to use template for bit of something other than a byte.
 * use: Bitof<uint16_t>(12)  => 16 bits equal to 1<<12.
 * */
template<typename Intish> constexpr Intish Bitof(const unsigned bitnum) {
  if (bitnum / 8 >= sizeof(Intish)) {
    return 0;
  }
  return Intish(1) << bitnum;
}

/** when the bit is known at compile time 
    Bit<uint32_t,5>() is a function call, not an object construction.
    uint8_t enableBit = Bit<typeof(enableBit),4>();
    need to try auto enableBit = Bit<uint8_t,4>(); on all potential compilers of interest. I expect avr to not handle this, too ancient a version of the language.
*/
template<typename Intish, unsigned bitnum> constexpr Intish Bit() {
  static_assert(bitnum / 8 < sizeof(Intish), "type does not have that many bits");
  return Intish(1) << bitnum;
}

/** @returns false for bits that do not exist. */
template<typename Intish> constexpr bool getBit(const unsigned bitnum, const Intish data) {
  if (bitnum / 8 > sizeof(Intish)) {
    return false;
  }
  return (data & Bitof<Intish>(bitnum)) != 0;
}

template<unsigned bitnum, typename Intish> constexpr bool getBit(const Intish data) {
  return (data & Bit<Intish, bitnum>()) != 0;
}

template<typename Intish> constexpr void setBit(const unsigned bitnum, Intish &data, bool setit) {
  if (bitnum / 8 <= sizeof(Intish)) {
    if (setit) {
      data |= Bitter<Intish>(bitnum);
    } else {
      data &= ~Bitter<Intish>(bitnum);
    }
  }
}

template<unsigned bitnum, typename Intish> constexpr void setBit(Intish &data, bool setit) {
  static_assert(bitnum / 8 < sizeof(Intish));
  if (bitnum / 8 <= sizeof(Intish)) {
    if (setit) {
      data |= Bitter<Intish>(bitnum);
    } else {
      data &= ~Bitter<Intish>(bitnum);
    }
  }
}

/**
 * e.g. for bits 6..3 0x78   Mask<6,3>::places
 * for masking after shifting field to line up lsb use Mask<4,3>::shifted  which will be 0x03
 * optional 3rd template argument must sometimes be provided by user to stifle compiler pickiness
 * */
template<unsigned msbit, unsigned lsbit = msbit, unsigned outof = msbit + 1> struct Mask {
  static_assert(msbit >= lsbit, "operand order is msbit,lsbit ");//require ordered to simplify the code
  //by using unsigned we don't need to check for negatives
  //we use unsigned as very few of the numbers are signed and the math really does not depend upon signedness
  using RawType = typename Unsigned<outof>::type;
  enum : RawType {
    width = msbit - lsbit + 1 , //check: needs to be 1 when msbit==lsbit
    //the casting for 'shifted' is needed as no matter what I do the compiler reinterprets the data as a signed int in its computations for enums.
    shifted = RawType(~(RawType(~0U) << width)) ,  //check: should be 1 == 1<<0 when msbit==lsbit, width =1 1<<1 = 2, -1 = 1, check should be -1 when msbit=numbits in type-1, and lsbit=0
    places = shifted << lsbit // suitable for field insertion mask
  };
};

/** @returns a substring of bits from @param data, returning the same sized value even if it could be a smaller type
 * this version only works for unsigned data. If you want to extract a signed field use a struct with bit fields instead of this.
 * */
template<unsigned msbit, unsigned lsbit, typename Intish> constexpr Intish getBits(const Intish data) {
  static_assert(msbit / 8 < sizeof(Intish));//does the data type even have that many bits?

  return (data >> lsbit) & Mask<msbit, lsbit>::shifted;
}

template<typename Intish> constexpr uint8_t getByte(const unsigned bytenum, const Intish data) {
  if (bytenum > sizeof(Intish)) {
    return 0;//shifted into nothingness
  }
  return data >> (8 * bytenum);
}

template<unsigned bytenum, typename Intish> constexpr uint8_t getByte(const Intish data) {
  static_assert(bytenum < sizeof(Intish), "operand doesn't have that many bytes");
  return data >> (8 * bytenum);
}

/** read and writes like a bit */
class BitAlias {
  const unsigned offset;
  uint8_t &wrapped;
public:

  BitAlias &operator=(bool setme) {
    setBit(offset, wrapped, setme);
    return *this;
  }

  operator bool() const {
    return getBit(offset, wrapped);
  }

  BitAlias(unsigned char &datum, unsigned bitnum) : offset(bitnum), wrapped(datum) {
  }
};


