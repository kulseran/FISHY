/**
 * compile-time type identifiers
 */
#ifndef FISHY_TYPEID_H
#define FISHY_TYPEID_H

#include <CORE/types.h>

#ifndef USE_BUILTIN_TYPEID
#  define USE_BUILTIN_TYPEID (1)
#endif

namespace core {
namespace util {

/**
 * Retrieve a unique type id for a given tType.
 */
#if USE_BUILTIN_TYPEID
template < typename tType >
Signature64 TypeId() {
  return (Signature64) typeid(tType).hash_code();
}
#else
Signature64 getUniqueInstanceSignature();

template < typename tType >
Signature64 TypeId() {
  static const Signature id = getUniqueInstanceSignature();
  return id;
}
#endif

} // namespace util
} // namespace core

#endif
