/* Copyright 2021 by Andy Heilveil, github/980f, on 12/19/21.
This was created to clean up the horrendous mess that was ST's vl53l0x series laser based range sensors.

They had macros for doing the following in C code, and I found errors all over the place due to careless use of them.
Most commonly they truncated multi-integer math, making the fractional part useless.

*/

#pragma once

#include "bitmanipulators.h"
#if __has_include(<cstdint>)
#include <cstdint>  //uint8_t in a few places
#else
#include "stdint.h"
#include "numlimits.h"
#endif

#include "intsizer.h"

/////////////////////// utilities

/** @returns value squared, saturated (if greater than or equal to half as many bits then it would overflow)
 * */
template<typename Intish> Intish squared_s(Intish num) {
  //test for overflow and saturate instead OR extend the return type to twice as many bits
  //todo: below only works for unsigned types, signed can return an unsigned value to hold on to some range
  if (num >= (Bitter<Intish>(std::numeric_limits<Intish>::digits / 2))) {//32 bit code used >65535 for the check, here we use >=65536
    //overflows
    return std::numeric_limits<Intish>::max();
  }
  return num * num;
}

/** @returns @param num divided by @param denom, rounded

   @note: divide by zero returns ZERO  due to frequency of ternaries doing that in first project to use this code.
   If you  don't like that then add  ,IntishOver dvz=0 */
template<typename IntishOver, typename IntishUnder> IntishOver roundedDivide(IntishOver num, IntishUnder denom) {
  return denom ? (num + (denom >> 1)) / denom : 0;//using 0 for divide by zero as a local preference. IE the first places that actually checked for /0 used 0 as the ratio.
}

/** @returns value divided by 2^ @param powerof2 rounded rather than truncated*/
template<typename IntishOver> IntishOver roundedScale(IntishOver num, unsigned powerof2) {
  if (powerof2 == 0) {//avert UB of shift in rounding term computed as -1
    return num;
  }
  if (powerof2 >= 8 * sizeof(IntishOver)) {//avert UB of shifting more than bits in the datum
    return 0;
  }
  return (num + (1 << (powerof2 - 1))) >> powerof2;//# fully parenthesized for clarity ;)
}

/** @returns @param num divided by 1000, rounded.
   this is somewhat the reverse of FixPoint millis which multiplies the nominal value by 1000 and rounds to integer */
template<typename IntishOver> IntishOver kilo(IntishOver num) {
  return roundedDivide(num, 1000);
}

//deprecated due to weird operand order and all uses were removed.
///** @returns @param value multiplied by @param shift power of 2
// * @note operand order, shift precedes value  */
//template<typename Intish> constexpr Intish boost(unsigned shift, Intish value) {
//  return value << shift;
//}

/** alters @param value to be no greater than @param max 
  @returns whether value was altered.
*/
template<typename Intish> constexpr bool lessen(Intish &value, Intish max) {
  if (value > max) {
    value = max;
    return true;
  }
  return false;
}

/** alters @param value to be no greater than @param max 
  @returns whether value was altered.
*/
template<typename Intish> constexpr bool ensure(Intish &value, Intish min) {
  if (value < min) {
    value = min;
    return true;
  }
  return false;
}

/** ceil(num/denom), number of bins needed in a histogram where num is the range and denom the quantum */
template<typename IntishOver, typename IntishUnder> IntishOver binsRequired(IntishOver num, IntishUnder denom) {
  return (num + denom - 1) / denom;
}

/** use where fractional values are expected
   Formerly the gyrations in here were explicitly repeated in very many places, each having to be checked for rogue tile error.
   Now we know that adding half the denominator before dividing in order to round always uses the same value for that halving, in one spot it did not!
   Given a floating point value f it's .16 bit point is (int)(f*(1<<16))
 * */
template<unsigned whole, unsigned fract> struct FixPoint {

  enum {
    size = (fract + whole)  //total number of bits needed to represent the value, used to pick the underlying int type.
  };
  
  using RawType = typename Unsigned<size>::type; //we use unsigned as very few of the numbers are signed and the math really does not depend upon signedness
  enum {
    mask = Mask<fract-1, 0, size>::places,   //to extract fraction bits from lsbs
    unity = Bitter<RawType>(fract), //equals 1.0F
    half = Bitter<RawType>(fract - 1)  // 0.5    
  };

  //for rounding to integer:
  using WholeType = typename Unsigned<whole>::type;

  ////////////////
  RawType raw;//the official type to which we are mapping www.fff conceptual values.

  operator RawType() const {
    return raw;
  }

  operator RawType &() {
    return raw;
  }

  /** convert to a float */
  explicit operator float() const {
    return float(raw >> fract) + float(raw & mask) / float(unity);
  }


  /**
     assigning with shifting and truncation or expansion as needed
   * */
  template<unsigned other_whole, unsigned other_fract> FixPoint(FixPoint<other_whole, other_fract> other) {
    //e.g: <16,16>other  to <9,7> this (uint16_t)((Value >> 9) & 0xFFFF)
    const int bitdiff = fract - other_fract;
    if constexpr (unsigned(size) >= unsigned(other.size)) {//we are expanding so inflate and shrunk
      raw = other.raw;//radix point is still at other_fract position
      if constexpr (bitdiff < 0) {//truncating some bits on the ls end
        raw >>= (-bitdiff);
      } else if constexpr (bitdiff > 0) {
        raw <<= (bitdiff);
      }
    } else { //shrinking so must align before truncating
      if constexpr (bitdiff < 0) {//truncating some bits on the ls end
        other.raw >>= (-bitdiff);
      } else if constexpr(bitdiff > 0) {
        other.raw <<= (bitdiff);
      }
      raw = other.raw;
    }
  }

/** java like zero init of anything not explicitly init. */
  FixPoint() : raw(0) {
  }

/** This is a debatable choice. It is used for copying bit patterns around, rather than operator=(int to be scaled to fixed) */
  template<typename Intish> explicit FixPoint(Intish stuff) {
    raw = stuff;
  }

/** This is a debatable choice. It is used for copying bit patterns around, rather than operator=(int to be scaled to fixed) */
 FixPoint &operator=(RawType pattern) {
    raw = pattern;
    return *this;
  }

  /** two ints init item to ratio, with boosting of numerator by @param boostit power of two which defaults to fract, which treats num as an integer value */
  template<typename IntishUp, typename IntishDown> constexpr FixPoint(IntishUp num, IntishDown denom, unsigned boostit = fract) {
    raw = num;//expand to internal bitwidth asap.
    boosted(boostit);
    if (denom != IntishDown(1)) {//do not round if denom is 1, which would only make a difference if boostit != fract
      divideby(denom);
    }
  }

  constexpr FixPoint(float eff) {
    if (eff < 0) { //need to see if this ever occurs or if all entities are strictly positive or checked by app.
      raw = 0;
    } else if (eff == 0.0) {//frequent enough to special case
      raw = 0;
    } else {
      raw = RawType(eff * unity);//todo: pick bits rather than actually multiply
    }
  }

  /** without this explicit cast operator doubles preferentially converted to uint32's instead of floats!*/
  constexpr FixPoint(double eff) {
    raw = eff > 0 ? RawType(eff * unity) : 0;
  }
  
  FixPoint &operator=(float eff) {
    raw = FixPoint(eff).raw;//borrow constructor
    return *this;
  }

  ///////////////////////////////////////
  /** this is logically dangerous but matches legacy use. Logically one would expect to assign to the whole part, ie shift up by fract. */
  template<unsigned other_whole, unsigned other_fract> FixPoint &operator=(FixPoint<other_whole, other_fract> other) {
    raw = static_cast<FixPoint>(other);//use constructor to do conversion as the vast majority were inline. Defining the targets as the appropriate type will let us drop macros that use the constructor.
    return *this;
  }
 
  //////////////////////////////////////

  /** @returns nearest integer to nominal value
    Failure in ST's code to do this rounding was common, often the adding of 'half' was omitted making the fract part somewhat a waste of time.
  */
  WholeType rounded() const {
    return (raw + half) >> fract;
  }

  /** @returns the raw value of this shrunk by 2^ @param  bits, rounding */
  RawType shrink(unsigned bits) const {
    return roundedScale(raw, bits);
  }

  /** @returns this after dividing by 2^ @param bits, rounding */
  FixPoint &shrunk(unsigned bits = fract) {
    raw = this->shrink(bits);
    return *this;
  }

  /** @returns value of this after multiplying by 2^ @param bits */
  RawType shiftup(unsigned bits) const {
    return raw << bits;
  }

  /** @returns this after dividing by 2^ @param bits, rounding */
  FixPoint &boosted(unsigned bits) {
    raw <<= bits;
    return *this;
  }

  RawType squared(unsigned reducebits = 0) const {
    return roundedScale(::squared_s(raw), reducebits);
  }

  FixPoint &square(unsigned reducebits = 0) {
    raw = squared(reducebits);
    return *this;
  }

  /** @returns this after ensuring that it is no greater than @param max */
  FixPoint &lessen(FixPoint max) {
    if (raw > max.raw) {
      /* Clip to prevent overflow. Will ensure safe max result. */
      raw = max.raw;
    }
    return *this;
  }

  /** @returns integer part after times 1000, rounded to integer. Might overflow. */
  RawType millis() const {
    return ((raw * 1000) + half) >> fract;
  }

/** todo: apply <=> once we are sure all processors have their compiler upgraded to c++20 */
  bool operator<(const FixPoint &rhs) const {
    return raw < rhs.raw;
  }

  bool operator>(const FixPoint &rhs) const {
    return raw > rhs.raw;
  }

  bool operator<=(const FixPoint &rhs) const {
    return raw <= rhs.raw;
  }

  bool operator>=(const FixPoint &rhs) const {
    return raw >= rhs.raw;
  }

  bool operator==(const FixPoint &rhs) const {
    return raw == rhs.raw;
  }

  bool operator!=(const FixPoint &rhs) const {
    return raw != rhs.raw;
  }

  RawType operator+(const FixPoint &rhs) {
    return raw + rhs.raw;
  }

  RawType operator-(const FixPoint &rhs) {
    return raw - rhs.raw;
  }

  RawType operator*(const FixPoint &rhs) {
    return raw * rhs.raw;
  }
  //force explicit roundedDivided

  FixPoint &operator+=(const FixPoint &rhs) {
    raw += rhs.raw;
    return *this;
  }

  FixPoint &operator-=(const FixPoint &rhs) {
    raw -= rhs.raw;
    return *this;
  }

  FixPoint &operator*=(const FixPoint &rhs) {
    raw *= rhs.raw;
    return *this;
  }

  RawType operator+(RawType raw_) {
    return raw + raw_;
  }

  RawType operator-(RawType raw_) {
    return raw - raw_;
  }

  RawType operator*(RawType raw_) {
    return raw * raw_;
  }
  //force explicit roundedDivided

  FixPoint &operator+=(RawType raw_) {
    raw += raw_;
    return *this;
  }

  FixPoint &operator-=(RawType raw_) {
    raw -= raw_;
    return *this;
  }

  FixPoint &operator*=(RawType raw_) {
    raw *= raw_;
    return *this;
  }

  /** rounding /= */
  template<typename Intish> FixPoint &divideby(Intish denom) {
    raw = roundedDivide(raw, denom);
    return *this;
  }
};

/** @returns value divided by 2^ @param powerof2 rounded rather than truncated*/
template<unsigned whole, unsigned fract> auto roundedScale(FixPoint<whole, fract> num, unsigned powerof2) -> typename decltype(num)::RawType {
  return ::roundedScale(num.raw, powerof2);//# fully parenthesized for clarity ;)
}

/** @returns @param num divided by @param denom, rounded */
template<unsigned whole, unsigned fract, typename IntishUnder>
auto roundedDivide(FixPoint<whole, fract> num, IntishUnder denom) -> typename decltype(num)::RawType {
  return ::roundedDivide(num.raw, denom);
}
