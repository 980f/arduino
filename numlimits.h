#pragma once

/* some compiler implementations are missing numeric_limits , we create a minimal set here adding instances at need
*/

#ifdef min
#include "minmax.h" //platform has a macro that interferes with std library, replace macro with real code.
#endif

namespace std {
  template<typename Intish> struct numeric_limits {
    static constexpr unsigned digits = sizeof(Intish) * 8;
    static constexpr bool is_signed = Intish(-1) < 0;//gcc trick
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
    static constexpr unsigned radix = 2;

    static constexpr Intish epsilon() noexcept {
      return 1; //980f thinks gcc flubbed this badly, it is the smallest amount you need to add to a loop variable to ensure it changes.
    }

    static constexpr Intish min() noexcept {
      //set sign bit for signed typtes
      return is_signed ? (Intish(1) << (digits - 1)) : 0;
    }
    //one less than
    static constexpr Intish max() noexcept {
      //strange but true: the subtract will wrap around from min to max.
      return is_signed ? min() - 1 : ~0;
    }

    static constexpr unsigned digits10 = 1 + unsigned(digits * 0.30103);//ceil(bits*log10(2))
  };

  //cheap way to remove const and volatile:
  template<typename _Tp> struct numeric_limits<const _Tp> : public numeric_limits<_Tp> {
  };
  template<typename _Tp> struct numeric_limits<volatile _Tp> : public numeric_limits<_Tp> {
  };
  template<typename _Tp> struct numeric_limits<const volatile _Tp> : public numeric_limits<_Tp> {
  };

  /** can't use template base as the storage for bool is not clearly a bit, sometimes it is a byte */
  template<> struct numeric_limits<bool> {
    static constexpr int digits = 1;
    static constexpr bool is_signed = false;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
    static constexpr int radix = 2;

    static constexpr bool epsilon() noexcept {
      return 1;
    }

    static constexpr bool min() noexcept {
      return false;
    }

    static constexpr bool max() noexcept {
      return true;
    }

    static constexpr int digits10 = 1;//gcc seems off by 1, might have to match it for portability
  };
}
