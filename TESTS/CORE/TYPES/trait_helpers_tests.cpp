#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/TYPES/trait_helpers.h>
#include <sstream>
#include <vector>

using core::types::has_bit_and;
using core::types::has_bit_or;
using core::types::has_bit_xor;
using core::types::has_equality;
using core::types::has_greater_equal;
using core::types::has_greater_than;
using core::types::has_inequality;
using core::types::has_left_shift;
using core::types::has_less_equal;
using core::types::has_less_than;
using core::types::has_right_shift;

REGISTER_TEST_CASE(testBasicOperators) {
  TEST(testing::assertTrue(has_bit_and< int, int >::value));
  TEST(testing::assertTrue(has_bit_or< int, int >::value));
  TEST(testing::assertTrue(has_bit_xor< int, int >::value));
  TEST(testing::assertTrue(has_equality< int, int >::value));
  TEST(testing::assertTrue(has_greater_equal< int, int >::value));
  TEST(testing::assertTrue(has_greater_than< int, int >::value));
  TEST(testing::assertTrue(has_inequality< int, int >::value));
  TEST(testing::assertTrue(has_less_equal< int, int >::value));
  TEST(testing::assertTrue(has_less_than< int, int >::value));
}

REGISTER_TEST_CASE(testStreamOperators) {
  TEST(testing::assertTrue(
      has_right_shift< std::stringstream, const int & >::value));
  TEST(testing::assertTrue(
      has_left_shift< std::stringstream, const int & >::value));
  TEST(testing::assertFalse(
      has_left_shift< std::stringstream, std::vector< int > >::value));
}
