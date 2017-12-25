/**
 * math_consts.h
 *
 * Constants and conversions
 */
#ifndef FISHY_MATH_CONSTS_H
#define FISHY_MATH_CONSTS_H

#include <CORE/types.h>

namespace core {
namespace math {

extern const f32 PI;

/**
 * Return the radian value of a degree angle.
 */
inline f32 deg(f32 d) { return (((d) / 180.0f) * PI); }

/**
 * Return the degree value of a radian angle.
 */
inline f32 rad(f32 r) { return (((r) / PI) * 180.0f); }

} // namespace math
} // namespace core

#endif
