/**
 * Converts one type to the lexical representation in another type.
 *   int someInt = 5;
 *   std::string someString;
 *   bool ret = lexical_cast(someInt, someString);
 *   ASSERT(ret);
 *   std::cout << someString << std::endl;
 *
 * Prints "5"
 */
#ifndef FISHY_LEXICAL_CAST_H
#define FISHY_LEXICAL_CAST_H

#include <CORE/BASE/status.h>
#include <CORE/TYPES/trait_helpers.h>
#include <CORE/types.h>

#include <iomanip>
#include <sstream>

namespace core {
namespace util {

namespace detail {
template < bool, bool >
struct CasterImpl {
  template < typename tDest, typename tSource >
  static Status lexical_cast(const tSource &a, tDest &b) {
    return Status(Status::BAD_ARGUMENT);
  }
};

template <>
struct CasterImpl< true, true > {
  template < typename tDest, typename tSource >
  static Status lexical_cast(const tSource &a, tDest &b) {
    std::stringstream caster;
    caster << a;
    caster >> b;
    return caster.fail() ? Status(Status::BAD_ARGUMENT) : Status::ok();
  }
};
} // namespace detail

/**
 * Cast tSource to a tDest. Requires that both tSource and tDest implement
 * operator <<(ostream &, tType)
 *
 * @return true if cast was a success
 */
template < typename tDest, typename tSource >
inline Status lexical_cast(const tSource &a, tDest &b) {
  return detail::CasterImpl<
      core::types::has_right_shift< std::stringstream, tDest & >::value,
      core::types::has_left_shift< std::stringstream, const tSource & >::
          value >::lexical_cast(a, b);
}

template < typename tDest >
inline Status lexical_cast(const tDest &a, tDest &b) {
  b = a;
  return Status::ok().ignoreErrors();
}

template <>
inline Status lexical_cast< f64, std::string >(const std::string &s, f64 &d) {
  d = (f64) atof(s.c_str());
  return Status::ok().ignoreErrors();
}

template <>
inline Status lexical_cast< f32, std::string >(const std::string &s, f32 &d) {
  d = (f32) atof(s.c_str());
  return Status::ok().ignoreErrors();
}

template <>
inline Status lexical_cast< u32, std::string >(const std::string &s, u32 &d) {
  d = (u32) atol(s.c_str());
  return Status::ok().ignoreErrors();
}

template <>
inline Status lexical_cast< s32, std::string >(const std::string &s, s32 &d) {
  d = (s32) atoi(s.c_str());
  return Status::ok().ignoreErrors();
}

template <>
inline Status lexical_cast< bool, std::string >(const std::string &s, bool &b) {
  if (s == "true" || s == "1") {
    b = true;
  } else {
    b = false;
  }
  return Status::ok().ignoreErrors();
}

} // namespace util
} // namespace core

#endif
