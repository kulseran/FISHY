/**
 * Functions to be called in test cases for checking the validity of tested
 * code's responses.
 */
#ifndef FISHY_TEST_ASSERTIONS_H
#define FISHY_TEST_ASSERTIONS_H

#ifndef TESTING
#  error may only be included in tests
#endif

#include <CORE/UTIL/lexical_cast.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

namespace testing {

/**
 * Kill the test
 */
void fail(const char * = nullptr);

/**
 * Kills the test if expr is not true.
 */
void assertTrue(const bool expr);

/**
 * Kills the test if expr is not false.
 */
void assertFalse(const bool expr);

/**
 * Kills the test if the actual != expected
 */
template < typename tTypeT, typename tTypeU >
inline void assertEquals(
    const tTypeT &actual,
    const tTypeU &expected,
    bool (*pred)(const tTypeT &, const tTypeU &) = nullptr) {
  if (pred != nullptr) {
    if (pred(actual, expected)) {
      return;
    }
  } else if (actual == expected) {
    return;
  }
  std::string actualstr;
  std::string expectedstr;
  if (!core::util::lexical_cast(actual, actualstr)) {
    actualstr = "<unprintable>";
  }
  if (!core::util::lexical_cast(expected, expectedstr)) {
    expectedstr = "<unprintable>";
  }
  const std::string msg = "Objects not equal. Actual [" + actualstr
                          + "]. Expected [" + expectedstr + "]";
  fail(msg.c_str());
}

inline bool floatCompare(const float &actual, const float &expected) {
  return std::fabs(actual - expected)
         < std::numeric_limits< float >().epsilon();
}

inline bool doubleCompare(const double &actual, const double &expected) {
  return std::fabs(actual - expected)
         < std::numeric_limits< float >().epsilon();
}

inline void assertEquals(const float &actual, const float &expected) {
  assertEquals(actual, expected, floatCompare);
}

inline void assertEquals(const double &actual, const double &expected) {
  assertEquals(actual, expected, doubleCompare);
}

/**
 * Kills the test if t not in u
 */
template < typename tTypeT, typename tTypeU >
inline void assertContains(const tTypeT &t, const tTypeU &u) {
  fail();
}

template <>
inline void assertContains< std::string, std::string >(
    const std::string &t, const std::string &u) {
  assertFalse(u.find(t) == u.npos);
}

/**
 * Kills the test if ptr is null
 */
void assertNotNull(const void *const ptr);

/**
 * Kills the test if ptr is not null
 */
void assertNull(const void *const ptr);

/**
 * Kills the test if the ptr is null
 */
template < typename tType >
void assertNotNull(const std::shared_ptr< tType > &ptr) {
  assertNotNull(ptr.get());
}

/**
 * Kills the test if ptr is not null
 */
template < typename tType >
void assertNull(const std::shared_ptr< tType > &ptr) {
  assertNull(ptr.get());
}

/**
 * Base exception thrown by test assertions, should only be caught by the test
 * harness.
 */
class TestAssertionException {
  public:
  TestAssertionException(const char *msg)
      : m_msg(msg), m_file("<unknown>"), m_func("<unknown>"), m_line(0) {}

  const char *what() { return m_msg; }
  const char *file() { return m_file; }
  const char *func() { return m_func; }
  const int line() { return m_line; }

  void addTrace(const char *file, const char *func, const int line) {
    m_file = file;
    m_func = func;
    m_line = line;
  }

  private:
  const char *m_msg;
  const char *m_file;
  const char *m_func;
  int m_line;
};

} // namespace testing

#  define TEST(assertExpr)                          \
    try {                                           \
      assertExpr;                                   \
    } catch (testing::TestAssertionException & e) { \
      e.addTrace(__FILE__, __FUNCTION__, __LINE__); \
      throw e;                                      \
    }

#endif
