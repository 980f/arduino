#pragma once

  template<typename Comparable> constexpr inline const Comparable& max(const Comparable& a, const Comparable& b){
      return  (a < b)?b: a;
    }

  template<typename Comparable> constexpr inline const Comparable& min(const Comparable& a, const Comparable& b){
      return  (a < b)?a: b;
    }

