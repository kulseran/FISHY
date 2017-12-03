/**
 * comparisonchain.inl
 *
 * inline implementations of comparisonchain.h
 */
#ifndef FISHY_COMPARISON_CHAIN_INL
#define FISHY_COMPARISON_CHAIN_INL

namespace core {
namespace util {

/**
 *
 */
inline ComparisonChain::ComparisonChain()
  : m_value(0) {
}

/**
 *
 */
inline int ComparisonChain::buildInt() const {
  return m_value;
}

/**
 *
 */
inline bool ComparisonChain::buildEqual() const {
  return m_value == 0;
}

/**
 *
 */
inline bool ComparisonChain::buildLessThan() const {
  return m_value < 0;
}

/**
 *
 */
inline bool ComparisonChain::buildGreaterThan() const {
  return m_value > 0;
}

/**
 *
 */
template <typename tType>
inline ComparisonChain &ComparisonChain::and(const tType &lhs, const tType &rhs) {
  if (buildEqual()) {
    m_value = compare(lhs, rhs);
  }
  return *this;
}

/**
 *
 */
template <typename tType>
inline ComparisonChain &ComparisonChain::or(const tType &lhs, const tType &rhs) {
  if (!buildEqual()) {
    m_value = compare(lhs, rhs);
  }
  return *this;
}

/**
 *
 */
inline ComparisonChain &ComparisonChain::and(const ComparisonChain &child) {
  and (child.buildInt());
  return *this;
}

/**
 *
 */
inline ComparisonChain &ComparisonChain::or(const ComparisonChain &child) {
  or (child.buildInt());
  return *this;
}

/**
 *
 */
inline ComparisonChain &ComparisonChain::and(const int &child) {
  if (buildEqual()) {
    m_value = child;
  }
  return *this;
}

/**
 *
 */
inline ComparisonChain &ComparisonChain::or(const int &child) {
  if (!buildEqual()) {
    m_value = child;
  }
  return *this;
}


/**
 *
 */
template <typename tIteratorLhs, typename tIteratorRhs>
inline ComparisonChain &ComparisonChain::andAll(
  const tIteratorLhs &lhsBegin,
  const tIteratorLhs &lhsEnd,
  const tIteratorRhs &rhsBegin,
  const tIteratorRhs &rhsEnd) {

  tIteratorRhs rhsItr = rhsBegin;
  tIteratorLhs lhsItr = lhsBegin;
  while (buildEqual() && rhsItr != rhsEnd && lhsItr != lhsEnd) {
    and (*lhsItr, *rhsItr);
    rhsItr++;
    lhsItr++;
  }
  and (std::distance(lhsItr, lhsEnd), std::distance(rhsItr, rhsEnd));
  return *this;
}

/**
 *
 */
template <typename tIteratorLhs, typename tIteratorRhs>
inline ComparisonChain &ComparisonChain::orAny(
  const tIteratorLhs &lhsBegin,
  const tIteratorLhs &lhsEnd,
  const tIteratorRhs &rhsBegin,
  const tIteratorRhs &rhsEnd) {

  tIteratorRhs rhsItr = rhsBegin;
  tIteratorLhs lhsItr = lhsBegin;
  while (rhsItr != rhsEnd && lhsItr != lhsEnd) {
    or (*lhsItr, *rhsItr);
    rhsItr++;
    lhsItr++;
  }
  and (std::distance(lhsItr, lhsEnd), std::distance(rhsItr, rhsEnd));
  return *this;
}

/**
 *
 */
inline ComparisonChain::ComparisonChain(int value)
  : m_value(value) {
}

/**
 *
 */
template <typename tType>
inline int ComparisonChain::compare(const tType &lhs, const tType &rhs) {
  if (lhs > rhs) { return 1; }
  else if (lhs < rhs) { return -1; }
  else { return 0; }
}

} // namespace util
} // namespace core

#endif
