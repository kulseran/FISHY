/**
 * Helper functions for handling arrays
 */
#ifndef FISHY_ARRAY_TYPES_H
#define FISHY_ARRAY_TYPES_H

/**
 * Compile time array length for valid types
 */
template <typename T, unsigned N>
inline unsigned ARRAY_LENGTH(const T(&arr)[N]) {
  (void)arr;
  return N;
}

/**
 * Macro'd version of above logic when a static result is required.
 */
#define ARRAY_LENGTH_M(n) (sizeof(n)/sizeof(n[0]))

#endif
