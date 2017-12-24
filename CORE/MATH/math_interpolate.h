/**
 * Templated interpolation functions
 */
#ifndef FISHY_MATH_INTERPOLATE_H
#define FISHY_MATH_INTERPOLATE_H

namespace core {
namespace math {

/**
 * Linear Interpolate from a->b by factor
 */
template < typename tType >
tType lerp(const tType &a, const tType &b, const f32 factor) {
  // Expand (a + ((b - a)*factor)) to insure functionality when (b-a) >> a or b
  const tType ablend = (tType)(a * factor);
  const tType bblend = (tType)(b * factor);
  return (a - ablend) + bblend;
}

/**
 * Cubic Hermite Spline interpolation
 * Interpolates along the spline <p0,t0>, <p1,t1>, <p2,t2>, <p3,t3> by T
 */
template < typename tType >
tType cubic_hermite(
    const tType &p0,
    const tType &p1,
    const tType &p2,
    const tType &p3,
    const f32 t0,
    const f32 t1,
    const f32 t2,
    const f32 t3,
    const f32 T) {
  const f32 tscale1 = 1.0f / (t2 - t0);
  const tType tangent1 = (p2 - p0) * tscale1;
  const f32 tscale2 = 1.0f / (t3 - t1);
  const tType tangent2 = (p3 - p1) * tscale2;

  const f32 T2 = T * T;
  const f32 T3 = T * T * T;

  const f32 h00 = 2.0f * T3 - 3.0f * T2 + 1.0f;
  const f32 h10 = T3 - 2.0f * T2 + T;
  const f32 h01 = -2.0f * T3 + 3.0f * T2;
  const f32 h11 = T3 - T2;

  return (h00 * p1 + h10 * tangent1 + h01 * p2 + h11 * tangent2);
}

/**
 * Preform a cubic_hermite interpolate over the point/time list <p,t> of size
 * sz >= 4 by factor T
 */
template < typename tType >
tType cubic_hermite(
    const tType *p, const f32 *t, const size_t sz, const f32 T) {
  ASSERT(p);
  ASSERT(t);
  ASSERT(sz >= 4);
  u32 index = 1;
  while ((index < (sz - 1)) && (t[index] < T)) {
    ++index;
  }
  const f32 Tinterp = (T - t[index]) / (t[index + 1] - t[index]);
  return cubic_hermite(
      p[index - 1],
      p[index + 0],
      p[index + 1],
      p[index + 2],
      t[index - 1],
      t[index + 0],
      t[index + 1],
      t[index + 2],
      Tinterp);
}

} // namespace math
} // namespace core

#endif
