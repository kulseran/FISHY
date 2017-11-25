/**
 * assert.h
 *
 * Macros to wrap up the standard "assert" call for all platforms.
 *   ASSERT(expression)
 * Macros for compile-time asserting values.
 *   COMPILE_TIME_ASSERT(expression)
 * Macros for wrapping expressions to evaluate only if asserts are on.
 *   IF_ASSERTS( do this )
 */
#ifndef FISHY_ASSERT_H
#define FISHY_ASSERT_H

#include <CORE/ARCH/platform.h>

#if defined(PLAT_WIN32)
#  include <crtdbg.h>
#  include <assert.h>
#elif defined(PLAT_LINUX)
#  include <assert.h>
#endif

#ifndef ASSERTS_ENABLED
#  if defined(DEBUG) || defined(_DEBUG)
#    define ASSERTS_ENABLED (1)
#  else
#    define ASSERTS_ENABLED (0)
#  endif
#endif

#ifndef COMPILE_ASSERTS_ENABLED
#  define COMPILE_ASSERTS_ENABLED (1)
#endif

/**
 * COMPILE_TIME_ASSERT(expression); will cause a compile-time error if expression does not
 *     evaluate to true.
 */
#if COMPILE_ASSERTS_ENABLED
#  if __cpp_static_assert
#    define COMPILE_TIME_ASSERT(expr)    static_assert(expr, "bad assert")
#  else
#    define COMPILE_TIME_ASSERT(expr)    typedef char UNIQUE_MACRO_NAME[(expr) != 0 ? 1 : -1]
#ifndef MAKE_NAME2
#    define UNIQUE_MACRO_NAME            MAKE_NAME(__LINE__, __COUNTER__)
#    define MAKE_NAME(line,counter)              MAKE_NAME2(line,counter)
#    define MAKE_NAME2(line,counter)             cta_failurecond_ ## line ## counter
#endif
#  endif
#else
#  define COMPILE_TIME_ASSERT(expr)
#endif

#if defined(PLAT_WIN32) || defined(_WIN32)
#  define ASSERT_BASE(expr) _ASSERT(expr)
#elif defined(PLAT_LINUX) || defined(linux)
#  define ASSERT_BASE(expr) { assert(expr); }
#else
#  error unsupported platform
#endif

/**
 * ASSERT_ALWAYS(); Will force the program to crash, even in release.
 */
#ifdef PLAT_WIN32
#  if defined(DEBUG) || defined(_DEBUG)
#    define ASSERT_ALWAYS() do { abort(); } while(0)
#  else
#    define ASSERT_ALWAYS() do { abort(); } while(0)
#  endif
#else
#  error unsupported platform
#endif

/**
 * ASSERT(expr); Will force the program to stop in the debugger
 *     if expr does not evaluate to true
 */
#if ASSERTS_ENABLED
#  define ASSERT(expr) ASSERT_BASE(expr)
#  define IF_ASSERTS(expr) expr
#  define IF_NOT_ASSERTS(expr)
#else
#  define ASSERT(expr)
#  define IF_ASSERTS(expr)
#  define IF_NOT_ASSERTS(expr) expr
#endif

#endif
