/**
 * Utility functions to limit numbers to a range
 */
#ifndef FISHY_MATH_LIMIT_H
#define FISHY_MATH_LIMIT_H

#include <CORE/types.h>

#include <algorithm>

namespace core {
namespace math {

/**
 * Clamp input between min and max.
 */
template < typename tType >
inline tType clamp(const tType &val, const tType &min, const tType &max) {
  return std::max(std::min(val, max), min);
}

/**
 * sudonym "min(val, max)"
 * Clamps the value to be no larger than max
 */
template < typename tType >
inline tType clamp_high(const tType &val, const tType &max) {
  return std::min(val, max);
}

/**
 * sudonym "max(val, min)"
 * Clamps the value to be no less than min
 */
template < typename tType >
inline tType clamp_low(const tType &val, const tType &min) {
  return std::max(val, min);
}

/**
 * Wrap around the range [min, max)
 */
inline f32 wrapf(const f32 &val, const f32 &min, const f32 &max) {
  if (val >= min && val < max) {
    return val;
  }
  const f32 range = max - min;

  if (val >= max) {
    const f32 bottom = val - min;
    const f32 dist = bottom / range;
    const u32 clamp = (u32) dist;
    return range * (dist - (f32) clamp) + min;
  } else {
    const f32 bottom = max - val;
    const f32 dist = bottom / range;
    const u32 clamp = (u32) dist;
    return max - range * (dist - (f32) clamp);
  }
}

/**
 * Check that two floats are equal within epislon
 */
inline bool
float_equal_epislon(const f32 &v, const f32 &u, const f32 &e = 0.0001f) {
  return ((v - e) < u && (v + e) > u);
}

/**
 * Check that two floats are equal with relative threshold
 */
inline bool
float_relative_equal(const f32 &v, const f32 &u, const f32 &e = 0.0001f) {
  if (float_equal_epislon(v, u, e)) {
    return true;
  }
  if (v == 0.0 || u == 0.0) {
    return false;
  }
  float error;
  if (u > v && -u < v) {
    error = (v - u) / u;
  } else {
    error = (v - u) / v;
  }
  return (error < e && -error < e);
}

/**
 * @return -1.0 or 1.0 depending on the sign of the input
 */
inline s32 sign(const s32 c) {
  return (c < 0) ? -1 : 1;
}

/**
 * @return -1.0 or 1.0 depending on the sign of the input
 */
inline f32 signf(const f32 c) {
  return (c < 0.0f) ? -1.0f : 1.0f;
}

} // namespace math
} // namespace core

#endif
