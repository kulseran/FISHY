#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/MATH/math_limit.h>
#include <CORE/types.h>

using core::math::clamp;
using core::math::clamp_high;
using core::math::clamp_low;
using core::math::float_equal_epislon;
using core::math::float_relative_equal;
using core::math::wrapf;

REGISTER_TEST_CASE(testClampHigh) {
  TEST(testing::assertEquals(clamp_high(10, 5), 5));
  TEST(testing::assertEquals(clamp_high(10, 15), 10));
}

REGISTER_TEST_CASE(testClampLow) {
  TEST(testing::assertEquals(clamp_low(10, 5), 10));
  TEST(testing::assertEquals(clamp_low(10, 15), 15));
}

REGISTER_TEST_CASE(testClamp) {
  TEST(testing::assertEquals(clamp(0, 5, 15), 5));
  TEST(testing::assertEquals(clamp(10, 5, 15), 10));
  TEST(testing::assertEquals(clamp(20, 5, 15), 15));
}

REGISTER_TEST_CASE(testWrap) {
  TEST(testing::assertEquals(wrapf(0.0f, 5.0f, 15.0f), 10.0f));
  TEST(testing::assertEquals(wrapf(10.0f, 5.0f, 15.0f), 10.0f));
  TEST(testing::assertEquals(wrapf(20.0f, 5.0f, 15.0f), 10.0f));
}

REGISTER_TEST_CASE(testEpislonEqual) {
  TEST(testing::assertTrue(float_equal_epislon(10.0f, 10.0f)));
  TEST(testing::assertTrue(float_equal_epislon(10.0f, 10.01f, 0.1f)));
  TEST(testing::assertFalse(float_equal_epislon(10.0f, 10.01f, 0.001f)));
}

REGISTER_TEST_CASE(testRelativeEqual) {
  TEST(testing::assertTrue(float_relative_equal(10.0f, 10.0f)));
  TEST(testing::assertTrue(float_relative_equal(10.0f, 10.01f, 0.1f)));
  TEST(testing::assertFalse(float_relative_equal(10.0f, 10.01f, 0.0001f)));
}
