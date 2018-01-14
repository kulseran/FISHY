#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/UTIL/typeid.h>
#include <CORE/types.h>

using core::util::TypeId;

typedef int tTestType;

REGISTER_TEST_CASE(testUniqueIds) {
  TEST(testing::assertTrue(TypeId< int >() == TypeId< int >()));
  TEST(testing::assertTrue(TypeId< int >() != TypeId< long >()));
  TEST(testing::assertTrue(TypeId< int >() == TypeId< tTestType >()));
}
