/**
 * bitset.h
 *
 * Type-safe bitset
 */
#ifndef FISHY_BITSET_H
#define FISHY_BITSET_H

#include <CORE/types.h>

namespace core {
namespace types {

/**
 * BitSet< type > should be created from an enumeration setup like:
 * struct type {
 *   enum e {
 *     ...,
 *     COUNT
 *   }
 * }
 *
 * It will create a minimal sized object that contains sufficient bits to treat
 * all the values of e as bits.
 */
template < typename eEnumType >
class BitSet {
  public:
  static constexpr unsigned int bytes_needed(int bits) {
    return (bits & 7) ? ((bits >> 3) + 1) : (bits >> 3);
  }

  enum { VEC_SIZE = bytes_needed(eEnumType::COUNT) };

  BitSet();

  /**
   * Set a bit.
   */
  void set(const typename eEnumType::type bit);

  /**
   * Clear a bit.
   */
  void unset(const typename eEnumType::type bit);

  /**
   * @return true if a bit is set.
   */
  bool isSet(const typename eEnumType::type bit) const;

  /**
   * @return the logical AND of two BitSet
   */
  BitSet operator&(const BitSet &other) const;

  /**
   * @see isSet
   * @return the logical AND with a given bit
   */
  bool operator&(const typename eEnumType::type val) const;

  /**
   * @return the logical OR of two BitSet
   */
  BitSet operator|(const BitSet &other) const;

  /**
   * @see set
   * @return the logical OR with a given bit
   */
  BitSet operator|(const typename eEnumType::type val) const;

  /**
   * Logical AND setter
   */
  BitSet &operator&=(const BitSet &other);

  /**
   * Logical AND setter
   */
  BitSet &operator&=(const typename eEnumType::type val);

  /**
   * Logical OR setter
   */
  BitSet &operator|=(const BitSet &other);

  /**
   * Logical OR setter
   */
  BitSet &operator|=(const typename eEnumType::type val);

  /**
   * Logical bitwise NOT
   */
  BitSet operator~() const;

  /**
   * @return true if any bit is set, false otherwise
   */
  operator bool() const;

  bool operator==(const BitSet< eEnumType > &other) const;

  private:
  u8 m_bits[VEC_SIZE];

  inline u32 index(const typename eEnumType::type bit) const;
  inline u32 shift(const typename eEnumType::type bit) const;
};

} // namespace types
} // namespace core

#  include "bitset.inl"

#endif
