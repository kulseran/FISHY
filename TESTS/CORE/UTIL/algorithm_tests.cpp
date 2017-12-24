#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/UTIL/algorithm.h>

#include <vector>

using core::util::erase_if;
using core::util::first_non_null;

/**
 * Test predicate which returns true for even numbers.
 */
class EvenPredicate {
  public:
  bool operator()(u32 i) const { return (i % 2) == 0; }
};

REGISTER_TEST_CASE(testEraseIf) {
  std::vector< int > elements;
  for (u32 i = 0; i < 100; ++i) {
    elements.push_back(i);
  }

  erase_if(elements, EvenPredicate());

  TEST(testing::assertEquals(50, elements.size()));
  for (u32 i = 0; i < 50; ++i) {
    TEST(testing::assertTrue((elements[i] % 2) == 1));
  }
}

REGISTER_TEST_CASE(testFirstNonNull) {
  int b = 0;
  int *pA = nullptr;
  int *pB = &b;
  int *pC = nullptr;

  TEST(testing::assertEquals(first_non_null(pA, pB), pB));
  TEST(testing::assertEquals(first_non_null(pB, pC), pB));
}
