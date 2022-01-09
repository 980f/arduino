#pragma once
/** for systems without a complete c++ standard library
 * allows forwarding
 *
 * */
namespace std {

  /// integral_constant for template hijinks
  template<typename Intish, Intish argv> struct integral_constant {
    static constexpr Intish value = argv;
    typedef Intish value_type;
    typedef integral_constant<Intish, argv> type;

    constexpr operator value_type() const noexcept {
      return value;
    }

    constexpr value_type operator()() const noexcept {
      return value;
    }
  };

  template<typename Intish, Intish argv> constexpr Intish integral_constant<Intish, argv>::value;

  typedef integral_constant<bool, true> true_type;
  typedef integral_constant<bool, false> false_type;

  template<typename> struct is_lvalue_reference : public false_type {
  };

  template<typename Anytype> struct is_lvalue_reference<Anytype &> : public true_type {
  };

  template<typename Anytype> struct remove_reference {
    typedef Anytype type;
  };

  template<typename Anytype> struct remove_reference<Anytype &> {
    typedef Anytype type;
  };

  template<typename Anytype> struct remove_reference<Anytype &&> {
    typedef Anytype type;
  };

  template<typename Anytype> constexpr Anytype &&forward(typename std::remove_reference<Anytype>::type &__t) noexcept {
    return static_cast < Anytype && > (__t);
  }

  template<typename Anytype> constexpr Anytype &&forward(typename std::remove_reference<Anytype>::type &&__t) noexcept {
    static_assert(!std::is_lvalue_reference<Anytype>::value, "can't forward lvalue reference as rvalue reference");
    return static_cast < Anytype && > (__t);
  }
}

