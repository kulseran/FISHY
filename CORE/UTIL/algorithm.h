/**
 * Inline additions to <algorithm> stl algorithms
 */
#ifndef FISHY_ALGORITHM_H
#define FISHY_ALGORITHM_H

namespace core {
namespace util {

/**
 * Erase elements from a container, if they match a predicate
 */
template < typename tContainer, typename tPredicate >
inline void erase_if(tContainer &items, const tPredicate &pred) {
  for (typename tContainer::iterator it = items.begin(); it != items.end();) {
    if (pred(*it)) {
      it = items.erase(it);
    } else {
      ++it;
    }
  }
}

/**
 * Returns the first non-null of the two input pointers.
 */
template < typename tType >
inline tType *first_non_null(tType *pA, tType *pB) {
  if (pA == nullptr) {
    return pB;
  }
  return pA;
}

/**
 * Returns the first non-null of the two input pointers.
 */
template < typename tType >
inline const tType *first_non_null(const tType *pA, const tType *pB) {
  if (pA == nullptr) {
    return pB;
  }
  return pA;
}

} // namespace util
} // namespace core

#endif
