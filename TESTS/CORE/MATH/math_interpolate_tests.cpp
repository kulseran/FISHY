#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/MATH/math_interpolate.h>
#include <CORE/types.h>

using core::math::cubic_hermite;
using core::math::lerp;

REGISTER_TEST_CASE(testLerp) {
  const int a = 1;
  const int b = 10;
  const int result = lerp(a, b, 0.5f);

  TEST(testing::assertEquals(result, 6));
}

REGISTER_TEST_CASE(testCubicHermite) {
  const float result = cubic_hermite(
      0.0f, 1.0f, 2.0f, 3.0f, 0.0f, 0.3333333f, 0.6666666f, 1.0f, 0.5f);

  TEST(testing::assertEquals(result, 1.5f));
}

REGISTER_TEST_CASE(testCubicHermiteArray) {
  const float points[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
  const float times[] = {0.0f,
                         0.3333333f,
                         0.6666666f,
                         1.0f,
                         1.3333333f,
                         1.6666666f,
                         2.00f,
                         2.3333333f};
  const float result = cubic_hermite(points, times, 8, 1.0f);

  TEST(testing::assertEquals(result, 4.0f));
}
