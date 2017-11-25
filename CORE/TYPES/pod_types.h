/**
 * Sized type renames
 */

#ifndef FISHY_POD_TYPES_H
#define FISHY_POD_TYPES_H

#include <stddef.h>
#include <stdint.h>

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t    s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

COMPILE_TIME_ASSERT(sizeof(u8) == 1);
COMPILE_TIME_ASSERT(sizeof(u16) == 2);
COMPILE_TIME_ASSERT(sizeof(u32) == 4);
COMPILE_TIME_ASSERT(sizeof(u64) == 8);
COMPILE_TIME_ASSERT(sizeof(s8) == 1);
COMPILE_TIME_ASSERT(sizeof(s16) == 2);
COMPILE_TIME_ASSERT(sizeof(s32) == 4);
COMPILE_TIME_ASSERT(sizeof(s64) == 8);

typedef float f32;
typedef double f64;
COMPILE_TIME_ASSERT(sizeof(f32) == 4);
COMPILE_TIME_ASSERT(sizeof(f64) == 8);

typedef u32 Signature;

#endif
