/**
 * Wrappers over processor intrinsics.
 */
#ifndef FISHY_INTRINSICS_H
#define FISHY_INTRINSICS_H

#include "platform.h"

#if defined(__clang__)
#  error clang compiler not supported
#elif defined(_MSC_VER)
#  define INLINE __forceinline
#  define FORCE_INLINE __forceinline
#  define NO_INLINE __declspec(noinline)
#  define ALIGN_STRUCT_BEGIN(_align) __declspec(align(_align))
#  define ALIGN_STRUCT_END(_align)
#elif defined(__GNUC__)
#  define INLINE __forceinline
#  define FORCE_INLINE __attribute__((always_inline))
#  define NO_INLINE __attribute__((noinline))
#  define ALIGN_STRUCT_BEGIN(_align)
#  define ALIGN_STRUCT_END(_align) __attribute__((aligned(_align)))
#else
#  error unknown compiler
#endif

#if defined(FISHY_NO_INTRINSICS)
#  include "intrinsics_none.inl"
#else
#  if defined(PLAT_WIN32) || defined(PLAT_LINUX)
#    include "intrinsics_pc.inl"
#  else
#    error unsupported platform
#  endif
#endif

#if !defined(VECF_128_LOAD) || !defined(VECF_128_ULOAD)
#  error missing intrinsics
#endif

#endif
