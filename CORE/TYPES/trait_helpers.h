/**
 * Various helpers for handling type traits
 */
#ifndef FISHY_TRAIT_HELPERS_H
#define FISHY_TRAIT_HELPERS_H

#include <functional>

namespace core {
namespace types {

template < class X, class Y, class Op >
struct op_valid_impl {
  template < class U, class L, class R >
  static auto test(int) -> decltype(
      std::declval< U >()(std::declval< L >(), std::declval< R >()),
      void(),
      std::true_type());

  template < class U, class L, class R >
  static auto test(...) -> std::false_type;

  using type = decltype(test< Op, X, Y >(0));
};

template < class X, class Y, class Op >
using op_valid = typename op_valid_impl< X, Y, Op >::type;

namespace notstd {

template < class _Ty = void >
struct left_shift { // functor for operator==
  typedef _Ty first_argument_type;
  typedef _Ty second_argument_type;
  typedef bool result_type;

  constexpr bool operator()(const _Ty &&_Left, const _Ty &&_Right)
      const { // apply operator << to operands
    return (_Left << _Right);
  }
};

template <>
struct left_shift< void > { // transparent functor for operator<<
  typedef int is_transparent;

  template < class _Ty1, class _Ty2 >
  constexpr auto operator()(_Ty1 &&_Left, _Ty2 &&_Right) const -> decltype(
      static_cast< _Ty1 & >(_Left) << static_cast< _Ty2 & >(
          _Right)) { // transparently apply operator<< to operands
    return (static_cast< _Ty1 & >(_Left) << static_cast< _Ty2 & >(_Right));
  }
};

template < class _Ty = void >
struct right_shift { // functor for operator==
  typedef _Ty first_argument_type;
  typedef _Ty second_argument_type;
  typedef bool result_type;

  constexpr bool operator()(const _Ty &&_Left, const _Ty &&_Right)
      const { // apply operator >> to operands
    return (_Left >> _Right);
  }
};

template <>
struct right_shift< void > { // transparent functor for operator<<
  typedef int is_transparent;

  template < class _Ty1, class _Ty2 >
  constexpr auto operator()(_Ty1 &&_Left, _Ty2 &&_Right) const -> decltype(
      static_cast< _Ty1 & >(_Left) << static_cast< _Ty2 & >(
          _Right)) { // transparently apply operator>> to operands
    return (static_cast< _Ty1 && >(_Left) >> static_cast< _Ty2 && >(_Right));
  }
};

} // namespace notstd

template < class X, class Y >
using has_equality = op_valid< X, Y, std::equal_to<> >;
template < class X, class Y >
using has_inequality = op_valid< X, Y, std::not_equal_to<> >;
template < class X, class Y >
using has_less_than = op_valid< X, Y, std::less<> >;
template < class X, class Y >
using has_less_equal = op_valid< X, Y, std::less_equal<> >;
template < class X, class Y >
using has_greater_than = op_valid< X, Y, std::greater<> >;
template < class X, class Y >
using has_greater_equal = op_valid< X, Y, std::greater_equal<> >;
template < class X, class Y >
using has_bit_xor = op_valid< X, Y, std::bit_xor<> >;
template < class X, class Y >
using has_bit_or = op_valid< X, Y, std::bit_or<> >;
template < class X, class Y >
using has_bit_and = op_valid< X, Y, std::bit_and<> >;
template < class X, class Y >
using has_left_shift = op_valid< X, Y, notstd::left_shift<> >;
template < class X, class Y >
using has_right_shift = op_valid< X, Y, notstd::right_shift<> >;

} // namespace types
} // namespace core

#endif
