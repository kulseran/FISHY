/**
 * Functions to replicate platform intrinsic operations where possible.
 *
 * TODO(kulseran): implement this file
 */
#ifndef FISHY_INTRINSICS_NONE_INL
#define FISHY_INTRINSICS_NONE_INL

ALIGN_STRUCT_BEGIN(16) struct INTRINSIC_VECF_128 {
  f32 _d[4];
} ALIGN_STRUCT_END(16);
ALIGN_STRUCT_BEGIN(16) struct INTRINSIC_VECI_128 {
  u32 _d[4];
} ALIGN_STRUCT_END(16);

typedef INTRINSIC_VECF_128 VECF_128;
typedef INTRINSIC_VECI_128 VECI_128;

#endif
