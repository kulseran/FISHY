/**
 * test_assertions.cpp
 *
 * implementations of test assertions
 */
#include "test_assertions.h"

#include <cstring>

namespace testing {

void assertTrue(bool expr) {
  if (!expr) {
    throw TestAssertionException("Expr not True");
  }
}

void assertFalse(bool expr) {
  if (expr) {
    throw TestAssertionException("Expr not False");
  }
}

void fail(const char *msg) {
  if (msg) {
    const size_t len = strlen(msg) + 1;
    char *pStr = new char[len];
    strncpy(pStr, msg, len);
    throw TestAssertionException(pStr);
  } else {
    throw TestAssertionException("test failed");
  }
}

void assertNull(const void *const ptr) {
  if (ptr != nullptr) {
    throw TestAssertionException("Pointer was un-expectedly nullptr");
  }
}

void assertNotNull(const void *const ptr) {
  if (ptr == nullptr) {
    throw TestAssertionException("Pointer was un-expectedly nullptr");
  }
}

} // namespace testing