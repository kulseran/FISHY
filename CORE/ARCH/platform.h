/**
 * Macros to define the platform this library is compiling under.
 *   PLAT_WIN32
 *   PLAT_LINUX
 */
#ifndef FISHY_PLATFORM_H
#define FISHY_PLATFORM_H

#include <cstddef>

#ifdef _WIN32
#  define PLAT_WIN32
#  define NOMINMAX
#  define WIN32_LEAN_AND_MEAN
#  define VC_EXTRALEAN
#elif defined(__linux__)
#  define PLAT_LINUX
#else
#  define PLAT_UNKNOWN
#  error unsupported platform
#endif

#if defined(_DEBUG) || defined(DEBUG)
#  define FISHY_DEBUG (1)
#else
#  define FISHY_DEBUG (0)
#endif

#endif
