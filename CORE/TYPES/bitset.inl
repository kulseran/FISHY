#ifndef FISHY_BITSET_INL
#define FISHY_BITSET_INL

namespace core {
namespace types {

/**
 *
 */
template < typename eEnumType >
inline u32
BitSet< eEnumType >::index(const typename eEnumType::type bit) const {
  return (static_cast< u32 >(bit) >> 3);
}

/**
 *
 */
template < typename eEnumType >
inline u32
BitSet< eEnumType >::shift(const typename eEnumType::type bit) const {
  return (static_cast< u32 >(bit) & 7);
}

/**
 *
 */
template < typename eEnumType >
inline BitSet< eEnumType >::BitSet() {
  std::fill(m_bits, m_bits + ARRAY_LENGTH(m_bits), 0);
}

/**
 *
 */
template < typename eEnumType >
inline void BitSet< eEnumType >::set(const typename eEnumType::type bit) {
  ASSERT(index(bit) < VEC_SIZE);
  m_bits[index(bit)] |= 1 << shift(bit);
}

/**
 *
 */
template < typename eEnumType >
inline void BitSet< eEnumType >::unset(const typename eEnumType::type bit) {
  ASSERT(index(bit) < VEC_SIZE);
  m_bits[index(bit)] &= ~(1 << shift(bit));
}

/**
 *
 */
template < typename eEnumType >
inline bool
BitSet< eEnumType >::isSet(const typename eEnumType::type bit) const {
  ASSERT(index(bit) < VEC_SIZE);
  return ((m_bits[index(bit)] & (1 << shift(bit))) != 0);
}

/**
 *
 */
template < typename eEnumType >
inline BitSet< eEnumType > BitSet< eEnumType >::
operator&(const BitSet &other) const {
  BitSet rVal;
  for (u32 i = 0; i < VEC_SIZE; ++i) {
    rVal.m_bits[i] = m_bits[i] & other.m_bits[i];
  }
  return rVal;
}

/**
 *
 */
template < typename eEnumType >
inline bool BitSet< eEnumType >::
operator&(const typename eEnumType::type val) const {
  return isSet(val);
}

/**
 *
 */
template < typename eEnumType >
inline BitSet< eEnumType > BitSet< eEnumType >::
operator|(const BitSet &other) const {
  BitSet rVal;
  for (u32 i = 0; i < VEC_SIZE; ++i) {
    rVal.m_bits[i] = m_bits[i] | other.m_bits[i];
  }
  return rVal;
}

/**
 *
 */
template < typename eEnumType >
inline BitSet< eEnumType > BitSet< eEnumType >::
operator|(const typename eEnumType::type val) const {
  BitSet rVal = *this;
  rVal.set(val);
  return rVal;
}

/**
 *
 */
template < typename eEnumType >
inline bool BitSet< eEnumType >::
operator==(const BitSet< eEnumType > &other) const {
  for (u32 i = 0; i < VEC_SIZE; ++i) {
    if (other.m_bits[i] != m_bits[i]) {
      return false;
    }
  }
  return true;
}

} // namespace types
} // namespace core

#endif
