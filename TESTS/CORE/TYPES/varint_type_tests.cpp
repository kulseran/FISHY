#include <TESTS/test_assertions.h>
#include <TESTS/testcase.h>

#include <CORE/types.h>

REGISTER_TEST_CASE(testValidZigzagEncodings) {
  for (int i = 0; i < 63; ++i) {
    u32 v = 1 << i;
    TEST(testing::assertEquals(encode_zigzag(decode_zigzag(v)), v));
  }
}
