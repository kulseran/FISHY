/**
 * checks.h
 *
 * Release build runtime assertions.
 *   CHECK(expression)
 *   CHECK_M(expression)
 */
#ifndef FISHY_CHECKS_H
#define FISHY_CHECKS_H

#include "asserts.h"
#include <CORE/BASE/logging.h>

/**
 * CHECK(expression); Will cause the program to crash if expression does not evaluate
 *     to true
 */
#define CHECK(expr) CHECK_M((expr), "Check failed.")

/**
 * CHECK_UNREACHABLE(); Will cause the program to crash if this code is reached.
 */
#define CHECK_UNREACHABLE() do { \
      Log(LL::Error) << "Should not have reached this line @ " << __FILE__ << ":" << __LINE__ << std::endl; \
      ASSERT_ALWAYS(); \
    } while(0)

/**
 * CHECK_INVALID_OPERATION(); Will cause the program to crash if this state is reached.
 */
#define CHECK_INVALID_OPERATION() do { \
      Log(LL::Error) << "Invalid Operation @ " << __FILE__ << ":" << __LINE__ << std::endl; \
      ASSERT_ALWAYS(); \
    } while(0)

/**
 * Temporary check used to signal that a function requires implemention.
 */
#define CHECK_NOT_IMPLEMENTED() CHECK_M(false, "This function requires implementation.")

/**
 * CHECK_M(expression, message); Will cause the program to crash with 'message' if expression
 *     does not evaluate to true
 */
#define CHECK_M(expr, msg) do { \
      if(!(expr)) { \
        Log(LL::Error) << msg << " @ " << __FILE__ << ":" << __LINE__ << std::endl; \
        ASSERT_ALWAYS(); \
      } \
    } while(0)

/**
 * RET_M(expression, message); Will 'return false' from a function if the expression
 *     does no evaluate to true
 */
#define RET_M(expr, msg) do { \
      if(!(expr)) { \
        Log(LL::Error) << msg << " @ " << __FILE__ << ":" << __LINE__ << std::endl; \
        return false; \
      } \
    } while(0)

/**
 * RET_ERRORCODE_M(expression, message; Will 'return expression' from a function if the
 *     expression does not evaluate to 0.
 */
#define RET_ERRORCODE_M(expr, msg) do { \
        const int ret = (expr); \
        if (ret != 0) { \
          Log(LL::Error) << msg << " @ " << __FILE__ << ":" << __LINE__ << std::endl; \
          return ret; \
        } \
      } while (0)

#endif
