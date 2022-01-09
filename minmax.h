#pragma once


/** these implementations don't rely upon the compiler's rules for promoting arguments in template matching. */
#undef min   

namespace std {
template <typename S1, typename S2> S1 min(S1 a, S2 b) {
   if constexpr(sizeof(S1) >= sizeof(S2)) {
    auto b1 = static_cast<S1>(b); //so incomparable types gives us just one error.
    if (a < b1) {
      return a;
    } else {
      return b1;
    }
  } else {
    return min(b, a);
  }
}

#undef max
template <typename S1, typename S2> S1 max(S1 a, S2 b) {
  if constexpr(sizeof(S1) >= sizeof(S2)) {
    auto bb = static_cast<S1>(b); //so incomparable types gives us just one error.
    if (a > bb) {
      return a;
    } else {
      return bb;
    }
  } else {
    return max(b, a);
  }
}
}
