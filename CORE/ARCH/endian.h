/**
 * Utility functions to swap bytes by endianess
 */
#ifndef FISHY_ENDIAN_H
#define FISHY_ENDIAN_H

#include <CORE/types.h>

/**
 * Determine the endianess of this platform
 */
#if defined(FISHY_USE_RUNTIME_ENDIAN_CHECK)
static const u32 ENDIAN_TEST_NO = 1;
#  define FISHY_IS_LITTLE_ENDIAN (((u8 *) &ENDIAN_TEST_NO)[0] == 0)
#else
#  if defined(__i386__) || defined(__alpha__) || defined(__ia64)       \
      || defined(__ia64__) || defined(_M_IX86) || defined(_M_IA64)     \
      || defined(_M_ALPHA) || defined(__amd64) || defined(__amd64__)   \
      || defined(_M_AMD64) || defined(__x86_64) || defined(__x86_64__) \
      || defined(_M_X64)
#    define FISHY_IS_LITTLE_ENDIAN (1)
#  else
#    define FISHY_IS_LITTLE_ENDIAN (0)
#  endif
#endif

namespace core {
namespace endian {

/**
 * Template that Swaps {@code sz} bytes.
 * Must be specialized below.
 */
template < u32 sz >
inline void endianSwapBytes(void *data) {
  COMPILE_TIME_ASSERT(sz == 0);
}

/**
 * Swap 1 byte object (no-op).
 */
template <>
inline void endianSwapBytes< 1 >(void *data) {
  (void) data;
}

/**
 * Swap 2 byte object.
 */
template <>
inline void endianSwapBytes< 2 >(void *data) {
  u16 *x = reinterpret_cast< u16 * >(data);
  *x = (((*x) << 8) & 0xFF00) | (((*x) >> 8) & 0x00FF);
}

/**
 * Swap 4 byte object.
 */
template <>
inline void endianSwapBytes< 4 >(void *data) {
  u32 *x = reinterpret_cast< u32 * >(data);
  *x = (((*x) << 24) & 0xFF000000) | (((*x) << 8) & 0x00FF0000)
       | (((*x) >> 8) & 0x0000FF00) | (((*x) >> 24) & 0x000000FF);
}

/**
 * Swap 8 byte object.
 */
template <>
inline void endianSwapBytes< 8 >(void *data) {
  u64 *x = reinterpret_cast< u64 * >(data);
  *x = (((*x) << 56) & 0xff00000000000000ULL)
       | (((*x) << 40) & 0x00ff000000000000ULL)
       | (((*x) << 24) & 0x0000ff0000000000ULL)
       | (((*x) << 8) & 0x000000ff00000000ULL)
       | (((*x) >> 8) & 0x00000000ff000000ULL)
       | (((*x) >> 24) & 0x0000000000ff0000ULL)
       | (((*x) >> 40) & 0x000000000000ff00ULL)
       | (((*x) >> 56) & 0x00000000000000ffULL);
}

/**
 * Perform a conversion to or from little endian.
 */
template < typename tType >
inline tType little(const tType &value) {
#if FISHY_IS_LITTLE_ENDIAN
  return value;
#else
  tType newValue = value;
  endianSwapBytes< sizeof(tType) >(&newValue);
  return newValue;
#endif
}

/**
 * Perform a conversion to or from big endian.
 */
template < typename tType >
inline tType big(const tType &value) {
#if FISHY_IS_LITTLE_ENDIAN
  tType newValue = value;
  endianSwapBytes< sizeof(tType) >(&newValue);
  return newValue;
#else
  return value;
#endif
}

/**
 * Return a boolean value indicating if the current platform is little-endian.
 */
inline bool isLittle() {
  return FISHY_IS_LITTLE_ENDIAN;
}

} // namespace endian
} // namespace core

#endif
