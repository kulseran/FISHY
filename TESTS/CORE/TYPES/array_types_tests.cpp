#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/types.h>

REGISTER_TEST_CASE(testValidArraySizer) {
  int testArray[16];
  TEST(testing::assertEquals(ARRAY_LENGTH(testArray), 16));
  TEST(testing::assertEquals(ARRAY_LENGTH_M(testArray), 16));
}
