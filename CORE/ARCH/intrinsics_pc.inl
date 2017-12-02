/**
 * Intrinsics for PC platforms
 */
#ifndef FISHY_INTRINSICS_PC_INL
#define FISHY_INTRINSICS_PC_INL

#if defined(PLAT_WIN32)
#  include <Windows.h>
#  include <emmintrin.h>
#  include <xmmintrin.h>
#elif defined(PLAT_LINUX)
#  include <sys/types.h>
#  include <xmmintrin.h>
#endif

typedef __m128 VECF_128;
typedef __m128i VECI_128;

// Data set operations
#  define VECF_128_LOAD(vf128) _mm_load_ps(vf128)
#  define VECF_128_ULOAD(vf128) _mm_loadu_ps(vf128)
#  define VECF_128_SETF(x, y, z, w) _mm_set_ps(w, z, y, x)
#  define VECF_128_SETI(x, y, z, w) _mm_castsi128_ps(_mm_set_epi32(w, z, y, x))
#  define VECF_128_SPLAT(value) _mm_set1_ps(value)

// Data get operations
#  define VECF_128_STORE(pfv4, vf128) _mm_store_ps(pfv4, vf128)
#  define VECF_128_USTORE(pfv4, vf128) _mm_storeu_ps(pfv4, vf128)
#  define VECF_128_GETX(x, vf128) _mm_store_ss(x, vf128)
#  define VECF_128_GETY(y, vf128) \
    _mm_store_ss(y, _mm_shuffle_ps(vf128, vf128, 1))
#  define VECF_128_GETZ(z, vf128) \
    _mm_store_ss(z, _mm_shuffle_ps(vf128, vf128, 2))
#  define VECF_128_GETW(w, vf128) \
    _mm_store_ss(w, _mm_shuffle_ps(vf128, vf128, 3))

// Math operations
#  define VECF_128_ADD(a, b) _mm_add_ps(a, b)
#  define VECF_128_SUB(a, b) _mm_sub_ps(a, b)
#  define VECF_128_MUL(a, b) _mm_mul_ps(a, b)
#  define VECF_128_DIV(a, b) _mm_div_ps(a, b)
#  define VECF_128_SQRT(a) _mm_sqrt_ps(a)
#  define VECF_128_RCP(a) _mm_rcp_ps(a)
#  define VECF_128_RSQRT(a) _mm_rsqrt_ps(a)
#  define VECF_128_MIN(a, b) _mm_min_ps(a, b)
#  define VECF_128_MAX(a, b) _mm_max_ps(a, b)

// Bitwise ops
#  define VECF_128_AND(a, b) _mm_and_ps(a, b)

// Shuffle operations
#  define VECF_128_SHUFFLE(s1, s2, mask) _mm_shuffle_ps(s1, s2, mask)
#  define VECF_128_SHUFFLEM(s1x, s1y, s2z, s2w) _MM_SHUFFLE(s1x, s1y, s2z, s2w)

// Atomics
#  if defined(PLAT_WIN32)
#    define ATOMIC_COMPARE_EXCHANGE_32(p, oldValue, newValue) \
      InterlockedCompareExchange(p, newValue, oldValue)
#    define ATOMIC_COMPARE_EXCHANGE_64(p, oldValue, newValue) \
      InterlockedCompareExchange64(p, newValue, oldValue)
#    define ATOMIC_INCREMENT(v) InterlockedIncrement(v)
#    define ATOMIC_DECREMENT(v) InterlockedDecrement(v)
#  elif defined(PLAT_LINUX)
#    define ATOMIC_COMPARE_EXCHANGE_32(p, oldValue, newValue) \
      __sync_val_compare_and_swap(p, oldValue, newValue)
#    define ATOMIC_COMPARE_EXCHANGE_64(p, oldValue, newValue) \
      __sync_val_compare_and_swap(p, oldValue, newValue)
#    define ATOMIC_INCREMENT(v) __sync_fetch_and_add(v, 1)
#    define ATOMIC_DECREMENT(v) __sync_fetch_and_sub(v, 1)
#  endif

#endif
