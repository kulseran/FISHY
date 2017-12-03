/**
 * comparisonchain.h
 *
 * Utility to simplify compaison, hash, and equality operators
 */
#ifndef FISHY_COMPARISON_CHAIN_H
#define FISHY_COMPARISON_CHAIN_H

namespace core {
namespace util {

/**
 * Encapsulation of multiple comparisons required to chain some
 * comparison and equality operators.
 */
class ComparisonChain {

  public:
    ComparisonChain();

    /**
     * Convert the answer for this comparison chain to an int.
     *
     * @return negative if less, zero if equal, positive if greater.
     */
    int buildInt() const;

    /**
     * Convert the answer for this comparison chain to a bool.
     *
     * @return true if equal, false otherwise.
     */
    bool buildEqual() const;

    /**
     * Convert the answer for this comparison chain to a bool.
     *
     * @return true if equal, false otherwise.
     */
    bool buildLessThan() const;

    /**
     * Convert the answer for this comparison chain to a bool.
     *
     * @return true if equal, false otherwise.
     */
    bool buildGreaterThan() const;

    /**
     * Logical And the result of this comparison to the chain.
     */
    template <typename tType>
    ComparisonChain & and (const tType &lhs, const tType &rhs);
    ComparisonChain & and (const ComparisonChain &child);
    ComparisonChain & and (const int &child);

    /**
     * Logical Or the result of this comparison to the chain.
     */
    template <typename tType>
    ComparisonChain & or (const tType &lhs, const tType &rhs);
    ComparisonChain & or (const ComparisonChain &child);
    ComparisonChain & or (const int &child);

    /**
     * Logical And the result of all these comparison to the chain.
     * Containers length difference is treased as {@link #and}(lengthLhs, lengthRhs).
     */
    template <typename tIteratorLhs, typename tIteratorRhs>
    ComparisonChain &andAll(
      const tIteratorLhs &lhsBegin,
      const tIteratorLhs &lhsEnd,
      const tIteratorRhs &rhsBegin,
      const tIteratorRhs &rhsEnd);

    /**
     * Logical Or the result of all these comparison to the chain.
     * Containers length difference is treased as {@link #and}(lengthLhs, lengthRhs).
     */
    template <typename tIteratorLhs, typename tIteratorRhs>
    ComparisonChain &orAny(
      const tIteratorLhs &lhsBegin,
      const tIteratorLhs &lhsEnd,
      const tIteratorRhs &rhsBegin,
      const tIteratorRhs &rhsEnd);

  private:
    /**
     * Comparison function, requires that objects used with
     * {@link ComparisonChain} implement operator '>' and '<'.
     *
     * @return negative if less, zero if equal, positive if greater.
     */
    template <typename tType>
    int compare(const tType &lhs, const tType &rhs);

    ComparisonChain(int);

    int m_value;
};

} // namespace util
} // namespace core

#include "comparisonchain.inl"

#endif
