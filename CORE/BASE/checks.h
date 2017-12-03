/**
 * Release build runtime assertions.
 *   CHECK(expression)
 *   CHECK_M(expression)
 */
#ifndef FISHY_CHECKS_H
#define FISHY_CHECKS_H

#include <CORE/BASE/asserts.h>

#include <iostream>

/**
 * CHECK(expression); Will cause the program to crash if expression does not
 * evaluate to true
 */
#define CHECK(expr) CHECK_M((expr), "Check failed.")

/**
 * CHECK_UNREACHABLE(); Will cause the program to crash if this code is reached.
 */
#define CHECK_UNREACHABLE()                                                \
  do {                                                                     \
    std::cerr << "Should not have reached this line @ " << __FILE__ << ":" \
              << __LINE__ << std::endl;                                    \
    ASSERT_ALWAYS();                                                       \
  } while (0)

/**
 * CHECK_INVALID_OPERATION(); Will cause the program to crash if this state is
 * reached.
 */
#define CHECK_INVALID_OPERATION()                                      \
  do {                                                                 \
    std::cerr << "Invalid Operation @ " << __FILE__ << ":" << __LINE__ \
              << std::endl;                                            \
    ASSERT_ALWAYS();                                                   \
  } while (0)

/**
 * Temporary check used to signal that a function requires implemention.
 */
#define CHECK_NOT_IMPLEMENTED() \
  CHECK_M(false, "This function requires implementation.")

/**
 * CHECK_M(expression, message); Will cause the program to crash with 'message'
 * if expression does not evaluate to true
 */
#define CHECK_M(expr, msg)                                                   \
  do {                                                                       \
    if (!(expr)) {                                                           \
      std::cerr << msg << " @ " << __FILE__ << ":" << __LINE__ << std::endl; \
      ASSERT_ALWAYS();                                                       \
    }                                                                        \
  } while (0)

#endif
